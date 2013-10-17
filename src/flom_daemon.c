/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include <stdio.h>
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_conns.h"
#include "flom_daemon.h"
#include "flom_errors.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON



int flom_daemon(const flom_config_t *config)
{
    enum Exception { PIPE_ERROR
                     , FORK_ERROR
                     , WAIT_ERROR
                     , CHILD_PID_ERROR
                     , READ_ERROR
                     , DAEMON_NOT_OK
                     , FORK_ERROR2
                     , SETSID_ERROR
                     , SIGNAL_ERROR
                     , FORK_ERROR3
                     , CHDIR_ERROR
                     , WRITE_ERROR
                     , FLOM_LISTEN_ERROR
                     , FLOM_ACCEPT_LOOP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int pipefd[2];
    int daemon = FALSE;
    int daemon_rc = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon\n"));
    TRY {
        pid_t pid;

        /* creating a communication pipe to understand when the daemon is
           ready to run */
        if (-1 == pipe(pipefd))
            THROW(PIPE_ERROR);
        FLOM_TRACE(("flom_daemon: pipefd[0]=%d, pipefd[1]=%d\n",
                    pipefd[0], pipefd[1]));
        
        /* creating a child process */
        if (-1 == (pid = fork())) {
            THROW(FORK_ERROR);
        } else if (0 != pid) {
            int status;
            pid_t child_pid;
            /* father process */
            FLOM_TRACE(("flom_daemon: started child process pid=" PID_T_FORMAT
                        "\n", pid));
            /* waiting termination: it's only a synchronization because
               while daemonizing the child will fork twice */
            if (-1 == (child_pid = wait(&status)))
                THROW(WAIT_ERROR);
            if (pid != child_pid) {
                FLOM_TRACE(("flom_daemon: something really wrong is happening;"
                            " pid=" PID_T_FORMAT ", child_pid=" PID_T_FORMAT
                            "\n", pid, child_pid));
                THROW(CHILD_PID_ERROR);
            }
            /* waiting the daemon is up and running */
            FLOM_TRACE(("flom_daemon: waiting daemon reason code...\n"));
            if (sizeof(daemon_rc) != read(pipefd[0], &daemon_rc,
                                          sizeof(daemon_rc)))
                THROW(READ_ERROR);
            FLOM_TRACE(("flom_daemon: daemon_rc=%d\n", daemon_rc));
            /* closing pipe (father) */
            close(pipefd[0]);
            close(pipefd[1]);
            if (FLOM_RC_OK != daemon_rc)
                THROW(DAEMON_NOT_OK);
        } else {
            pid_t pid;
            int i;
            int daemon_rc = FLOM_RC_OK;
            int listenfd = 0;

            /* switch state to daemonized */
            daemon = TRUE;
            /* child process, now daemonizing... */
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR2);
            } else if (0 != pid)
                exit(0);
            FLOM_TRACE(("flom_daemon: daemonizing... setsid()\n"));
            if (-1 == setsid())
                THROW(SETSID_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... signal()\n"));
            if (SIG_ERR == signal(SIGHUP, SIG_IGN))
                THROW(SIGNAL_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR3);
            } else if (0 != pid)
                exit(0);
            FLOM_TRACE(("flom_daemon: daemonizing... chdir()\n"));
            if (-1 == chdir("/"))
                THROW(CHDIR_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... umask()\n"));
            umask(0);
            FLOM_TRACE(("flom_daemon: daemonizing... close()\n"));
            /* close all but communication pipe */
            for (i = 0; i < 64; ++i)
                if (i != pipefd[0] && i != pipefd[1])
                    close(i);

            openlog("flom", LOG_PID, LOG_DAEMON);
            syslog(LOG_NOTICE, "flom daemon activated!");

            FLOM_TRACE_REOPEN(config->trace_file);
            FLOM_TRACE(("flom_daemon: now daemonized!\n"));

            /* activate service */
            daemon_rc = flom_listen(config, &listenfd);
            
            /* sending reason code to father process */
            if (sizeof(daemon_rc) != write(pipefd[1], &daemon_rc,
                                           sizeof(daemon_rc)))
                THROW(WRITE_ERROR);
            /* closing pipe (child) */
            close(pipefd[0]);
            close(pipefd[1]);
            /* checking flom_listed reason code */
            if (FLOM_RC_OK != daemon_rc)
                THROW(FLOM_LISTEN_ERROR);
            
            syslog(LOG_NOTICE, "flom_daemon: activated!");
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop(config, listenfd)))
                THROW(FLOM_ACCEPT_LOOP_ERROR);
            syslog(LOG_NOTICE, "flom_daemon: exiting");
            flom_listen_clean(config, listenfd);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case FORK_ERROR:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case WAIT_ERROR:
                ret_cod = FLOM_RC_WAIT_ERROR;
                break;
            case CHILD_PID_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case READ_ERROR:
                ret_cod = FLOM_RC_READ_ERROR;
                break;
            case DAEMON_NOT_OK:
                ret_cod = daemon_rc;
                break;
            case FORK_ERROR2:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case SETSID_ERROR:
                ret_cod = FLOM_RC_SETSID_ERROR;
                break;
            case SIGNAL_ERROR:
                ret_cod = FLOM_RC_SIGNAL_ERROR;
                break;
            case FORK_ERROR3:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case CHDIR_ERROR:
                ret_cod = FLOM_RC_CHDIR_ERROR;
                break;
            case WRITE_ERROR:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case FLOM_LISTEN_ERROR:
                ret_cod = daemon_rc;
                break;
            case FLOM_ACCEPT_LOOP_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    /* the function must not return control if in daemonized state */
    if (daemon)
        exit(0);
    return ret_cod;
}



int flom_listen(const flom_config_t *config,
                int *listenfd)
{
    enum Exception { SOCKET_ERROR
                     , UNLINK_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int fd = 0;
    
    FLOM_TRACE(("flom_listen\n"));
    TRY {
        struct sockaddr_un servaddr;
            
        if (-1 == (fd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        if (-1 == unlink(config->local_socket_path_name) &&
            ENOENT != errno)
            THROW(UNLINK_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_LOCAL;
        strcpy(servaddr.sun_path, config->local_socket_path_name);
        if (-1 == bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
            THROW(BIND_ERROR);
        if (-1 ==listen(fd, 100))
            THROW(LISTEN_ERROR);
        *listenfd = fd;
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case UNLINK_ERROR:
                ret_cod = FLOM_RC_UNLINK_ERROR;
                break;
            case BIND_ERROR:
                ret_cod = FLOM_RC_BIND_ERROR;
                break;
            case LISTEN_ERROR:
                ret_cod = FLOM_RC_LISTEN_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_listen/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_clean(const flom_config_t *config,
                      int listenfd)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_listen_clean\n"));
    TRY {
        if (-1 == close(listenfd)) {
            FLOM_TRACE(("flom_listen_clean: close errno=%d\n", errno));
        }
        if (-1 == unlink(config->local_socket_path_name)) {
            FLOM_TRACE(("flom_listen_clean: unlink errno=%d\n", errno));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_listen_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}


int flom_accept_loop(const flom_config_t *config, int listenfd)
{
    enum Exception { POLL_ERROR
                     , NETWORK_ERROR
                     , ACCEPT_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop\n"));
    TRY {
        struct pollfd poll_array[1];
        int ready_fd;
        int loop = TRUE;
        flom_conns_t fc;
        /* @@@ */
        flom_conns_init(&fc);
        
        while (loop) {
            poll_array[0].fd = listenfd;
            poll_array[0].events = POLLIN;
            ready_fd = poll(poll_array, 1, config->idle_time);
            FLOM_TRACE(("flom_accept_loop: ready_fd=%d\n", ready_fd));
            switch (ready_fd) {
                case -1:
                    THROW(POLL_ERROR);
                    break;
                case 0:
                    /* time out */
                    FLOM_TRACE(("flom_accept_loop: idle time exceeded %d "
                                "milliseconds, leaving...\n",
                                config->idle_time));
                    loop = FALSE;
                    break;
                case 1:
                    /* possible incoming client */
                    FLOM_TRACE(("flom_accept_loop: POLLIN=%d, POLLERR=%d, "
                                "POLLHUP=%d, POLLNVAL=%d\n",
                                poll_array[0].revents & POLLIN,
                                poll_array[0].revents & POLLERR,
                                poll_array[0].revents & POLLHUP,
                                poll_array[0].revents & POLLNVAL));
                    if (poll_array[0].revents &
                        (POLLERR | POLLHUP | POLLNVAL))
                        THROW(NETWORK_ERROR);
                    if (poll_array[0].revents & POLLIN) {
                        int conn_fd;
                        struct sockaddr_un cliaddr;
                        socklen_t clilen;
                        if (-1 == (conn_fd = accept(
                                       listenfd, (struct sockaddr *)&cliaddr,
                                       &clilen)))
                            THROW(ACCEPT_ERROR);
                        FLOM_TRACE(("flom_accept_loop: new client connected "
                                    "with fd=%d\n", conn_fd));
                        /* @@@ */
                    }
                    break;
                default:
                    THROW(INTERNAL_ERROR);
            }
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case ACCEPT_ERROR:
                ret_cod = FLOM_RC_ACCEPT_ERROR;
                break;
            case NETWORK_ERROR:
                ret_cod = FLOM_RC_NETWORK_EVENT_ERROR;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
