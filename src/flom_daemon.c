/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
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
#include "flom_locker.h"
#include "flom_msg.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON



int flom_daemon()
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
                     , CONNS_INIT_ERROR
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
            flom_conns_t conns;

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

            FLOM_TRACE_REOPEN(global_config.trace_file);
            FLOM_TRACE(("flom_daemon: now daemonized!\n"));

            /* activate service */
            if (FLOM_RC_OK != (ret_cod = flom_conns_init(&conns, AF_UNIX)))
                THROW(CONNS_INIT_ERROR);
            daemon_rc = flom_listen(&conns);
            
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
            
            openlog("flom", LOG_PID, LOG_DAEMON);
            syslog(LOG_NOTICE, "flom_daemon: activated!");
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop(&conns)))
                THROW(FLOM_ACCEPT_LOOP_ERROR);
            syslog(LOG_NOTICE, "flom_daemon: exiting");
            flom_listen_clean(&conns);
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
            case CONNS_INIT_ERROR:
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



int flom_listen(flom_conns_t *conns)
{
    enum Exception { SOCKET_ERROR
                     , UNLINK_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , CONNS_ADD_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int fd = 0;
    
    FLOM_TRACE(("flom_listen\n"));
    TRY {
        struct sockaddr_un servaddr;
            
        if (-1 == (fd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        if (-1 == unlink(global_config.local_socket_path_name) &&
            ENOENT != errno)
            THROW(UNLINK_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_LOCAL;
        strcpy(servaddr.sun_path, global_config.local_socket_path_name);
        if (-1 == bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
            THROW(BIND_ERROR);
        if (-1 ==listen(fd, 100))
            THROW(LISTEN_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               conns, fd, sizeof(servaddr),
                               (struct sockaddr *)&servaddr)))
            THROW(CONNS_ADD_ERROR);
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
            case CONNS_ADD_ERROR:
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



int flom_listen_clean(flom_conns_t *conns)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_listen_clean\n"));
    TRY {
        if (-1 == close(flom_conns_get_fd(conns, 0))) {
            FLOM_TRACE(("flom_listen_clean: close errno=%d\n", errno));
        }
        if (-1 == unlink(global_config.local_socket_path_name)) {
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


int flom_accept_loop(flom_conns_t *conns)
{
    enum Exception { CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , ACCEPT_LOOP_POLLIN_ERROR
                     , CONNS_CLOSE_ERROR
                     , NETWORK_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop\n"));
    TRY {
        int loop = TRUE;
        flom_locker_array_t lockers;

        flom_locker_array_init(&lockers);
        
        while (loop) {
            int ready_fd;
            nfds_t i, n;
            struct pollfd *fds;
            flom_conns_clean(conns);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            ready_fd = poll(flom_conns_get_fds(conns),
                            flom_conns_get_used(conns),
                            global_config.idle_time);
            FLOM_TRACE(("flom_accept_loop: ready_fd=%d\n", ready_fd));
            /* error on poll function */
            if (0 > ready_fd)
                THROW(POLL_ERROR);
            /* poll exited due to time out */
            if (0 == ready_fd) {
                FLOM_TRACE(("flom_accept_loop: idle time exceeded %d "
                            "milliseconds\n",
                            global_config.idle_time));
                /* @@@ check lockers to remove useless (write_pipe == NULL_FD)
                 */
                if (1 == flom_conns_get_used(conns)) {
                    FLOM_TRACE(("flom_accept_loop: only listener connection "
                                "is active, exiting...\n"));
                    loop = FALSE;
                }
                continue;
            }
            /* scanning file descriptors */
            n = flom_conns_get_used(conns);
            for (i=0; i<n; ++i) {
                fds = flom_conns_get_fds(conns);
                FLOM_TRACE(("flom_accept_loop: i=%d, fd=%d, POLLIN=%d, "
                            "POLLERR=%d, POLLHUP=%d, POLLNVAL=%d\n", i,
                            fds[i].fd,
                            fds[i].revents & POLLIN,
                            fds[i].revents & POLLERR,
                            fds[i].revents & POLLHUP,
                            fds[i].revents & POLLNVAL));
                if (fds[i].revents & POLLIN) {
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_pollin(
                                           conns, i, &lockers)))
                        THROW(ACCEPT_LOOP_POLLIN_ERROR);
                }
                if ((fds[i].revents & POLLHUP) && (0 != i)) {
                    FLOM_TRACE(("flom_accept_loop: client %i disconnected "
                                "before categorization!\n", i));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           conns, i)))
                        THROW(CONNS_CLOSE_ERROR);                       
                    continue;
                }
                if (fds[i].revents &
                    (POLLERR | POLLHUP | POLLNVAL))
                    THROW(NETWORK_ERROR);
            } /* for (i... */
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_SET_EVENTS_ERROR:
                break;
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case ACCEPT_LOOP_POLLIN_ERROR:
            case CONNS_CLOSE_ERROR:
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



int flom_accept_loop_pollin(flom_conns_t *conns, nfds_t id,
                            flom_locker_array_t *lockers)
{
    enum Exception { ACCEPT_ERROR
                     , CONNS_ADD_ERROR
                     , MSG_RETRIEVE_ERROR
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR
                     , ACCEPT_LOOP_TRANSFER_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_pollin\n"));
    TRY {
        struct pollfd *fds = flom_conns_get_fds(conns);
        FLOM_TRACE(("flom_accept_loop_pollin: id=%d, fd=%d\n",
                    id, fds[id].fd));
        if (0 == id) {
            /* it's a new connection */
            int conn_fd;
            struct sockaddr cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            if (-1 == (conn_fd = accept(
                           fds[id].fd, &cliaddr, &clilen)))
                THROW(ACCEPT_ERROR);
            FLOM_TRACE(("flom_accept_loop_pollin: new client connected "
                        "with fd=%d\n", conn_fd));
            if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                                   conns, conn_fd, clilen, &cliaddr)))
                THROW(CONNS_ADD_ERROR);
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            ssize_t read_bytes;
            struct flom_msg_s *msg;
            GMarkupParseContext *gmpc;
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                                   fds[id].fd, buffer, sizeof(buffer),
                                   &read_bytes)))
                THROW(MSG_RETRIEVE_ERROR);

            if (NULL == (msg = flom_conns_get_msg(conns, id)))
                THROW(CONNS_GET_MSG_ERROR);

            if (NULL == (gmpc = flom_conns_get_gmpc(conns, id)))
                THROW(CONNS_GET_GMPC_ERROR);
            
            if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                   buffer, read_bytes, msg, gmpc)))
                THROW(MSG_DESERIALIZE_ERROR);
            flom_msg_trace(msg);
            /* if the message is not valid the client must be terminated */
            if (FLOM_MSG_STATE_INVALID == msg->state) {
                FLOM_TRACE(("flom_accept_loop_pollin: message from client %i "
                            "is invalid, disconneting...\n", id));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR);            
            }
            /* check if the message is completely parsed and can be transferred
               to a slave thread (a locker) */
            if (FLOM_MSG_STATE_READY == msg->state)
                if (FLOM_RC_OK != (ret_cod = flom_accept_loop_transfer(
                                       conns, id, lockers)))
                    THROW(ACCEPT_LOOP_TRANSFER_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case ACCEPT_ERROR:
                ret_cod = FLOM_RC_ACCEPT_ERROR;
                break;
            case CONNS_ADD_ERROR:
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:
                break;
            case CONNS_GET_MSG_ERROR:
            case CONNS_GET_GMPC_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_CLOSE_ERROR:
            case ACCEPT_LOOP_TRANSFER_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_pollin/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_transfer(flom_conns_t *conns, nfds_t id,
                              flom_locker_array_t *lockers)
{
    enum Exception { NULL_OBJECT
                     , INVALID_VERB_STEP
                     , CONNS_GET_MSG_ERROR
                     , PIPE_ERROR
                     , G_THREAD_CREATE_ERROR
                     , WRITE_ERROR1
                     , WRITE_ERROR2
                     , CONNS_GET_CD_ERROR
                     , WRITE_ERROR3
                     , CONNS_TRNS_FD
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_locker_s *locker = NULL;
    int locker_allocated = TRUE;
    
    FLOM_TRACE(("flom_accept_loop_transfer\n"));
    TRY {
        gint i;
        int found = FALSE;
        int domain = 0, client_fd = NULL_FD;
        GThread *locker_thread = NULL;
        struct flom_msg_s *msg = NULL;
        const struct flom_conn_data_s *cd = NULL;
        /* check if there is a locker running for this request */
        if (NULL == lockers)
            THROW(NULL_OBJECT);
        /* check message */
        if (NULL == (msg = flom_conns_get_msg(conns, id)))
            THROW(CONNS_GET_MSG_ERROR);
        if (FLOM_MSG_VERB_LOCK != msg->header.pvs.verb ||
            FLOM_MSG_STEP_INCR != msg->header.pvs.step) {
            FLOM_TRACE(("flom_accept_loop_transfer: message verb (%d) and/or "
                        "step (%d) are not valid at this point;\n",
                        msg->header.pvs.verb, msg->header.pvs.step));
            THROW(INVALID_VERB_STEP);
        }
        /* is there a locker already active? */
        for (i=0; i<lockers->n; ++i) {
            locker = g_ptr_array_index(lockers->array,i);
            FLOM_TRACE(("flom_accept_loop_transfer: locker # %d is managing "
                        "resource '%s'\n", i, locker->resource_name));
            if (!g_strcmp0(locker->resource_name,
                           msg->body.lock_8.resource.name)) {
                FLOM_TRACE(("flom_accept_loop_transfer: found locker %d for "
                            "resource '%s'\n", i,
                            msg->body.lock_8.resource.name));
                found = TRUE;
                break;
            }
        } /* for (i=0; i<lockers->n; ++i) */
        if (!found) {
            /* start a new locker */
            locker = g_malloc0(sizeof(struct flom_locker_s));
            int pipefd[2];
            GError *error_thread;
            locker_allocated = TRUE;
            FLOM_TRACE(("flom_accept_loop_transfer: creating a new locker "
                        "for resource '%s'...\n",
                        msg->body.lock_8.resource.name));
            flom_locker_init(locker);
            locker->resource_name = g_strdup(msg->body.lock_8.resource.name);
            /* creating a communication pipe for the new thread */
            if (0 != pipe(pipefd))
                THROW(PIPE_ERROR);
            locker->read_pipe = pipefd[0];
            locker->write_pipe = pipefd[1];
            locker_thread = g_thread_create(flom_locker_loop, (gpointer)locker,
                                            FALSE, &error_thread);
            if (NULL == locker_thread) {
                FLOM_TRACE(("flom_accept_loop_transfer: "
                            "error_thread->code=%d, "
                            "error_thread->message='%s'\n",
                            error_thread->code, error_thread->message));
                g_free(error_thread);
                THROW(G_THREAD_CREATE_ERROR);
            } else {
                FLOM_TRACE(("flom_accept_loop_transfer: created thread %p\n",
                            locker_thread));
            }
            /* add this locker to the array of all lockers */
            flom_locker_array_add(lockers, locker);
        } else
            locker_thread = locker->thread;

        /* current connection must be transferred to the locker thread */
        client_fd = flom_conns_get_fd(conns, id);
        FLOM_TRACE(("flom_accept_loop_transfer: transferring connection %d "
                    "(fd %d) to thread %p using pipe %d\n", id, client_fd,
                    locker_thread, locker->write_pipe));
        /* send domain */
        domain = flom_conns_get_domain(conns);
        if (sizeof(domain) != write(
                locker->write_pipe, &domain, sizeof(domain)))
            THROW(WRITE_ERROR1);
        /* send connection file descriptor */
        if (sizeof(client_fd) != write(
                locker->write_pipe, &client_fd, sizeof(client_fd)))
            THROW(WRITE_ERROR2);
        /* send connection data */
        if (NULL == (cd = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        if (sizeof(struct flom_conn_data_s) != write(
                locker->write_pipe, cd, sizeof(struct flom_conn_data_s)))
            THROW(WRITE_ERROR3);
        /* set the connection as transferred to another thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_trns_fd(conns, id)))
            THROW(CONNS_TRNS_FD);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INVALID_VERB_STEP:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case CONNS_GET_MSG_ERROR:
                break;
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case G_THREAD_CREATE_ERROR:
                ret_cod = FLOM_RC_G_THREAD_CREATE_ERROR;
                break;
            case WRITE_ERROR1:
            case WRITE_ERROR2:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case CONNS_GET_CD_ERROR:
                break;
            case WRITE_ERROR3:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case CONNS_TRNS_FD:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        if (NONE != excp && locker_allocated) {
            /* clean-up locker */
            FLOM_TRACE(("flom_accept_loop_transfer: clean-up due to excp=%d\n",
                        excp));
            g_free(locker->resource_name);
            g_free(locker);
        }
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_transfer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

