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
            flom_conns_init(&conns, AF_UNIX);
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
                               (struct sockaddr *)&servaddr, TRUE)))
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
        flom_conns_free(conns);
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
    enum Exception { CONNS_CLEAN_ERROR
                     , CONNS_GET_FDS_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , ACCEPT_LOOP_CHKLOCKERS_ERROR1
                     , NEGATIVE_NUMBER_OF_LOCKERS_ERROR1
                     , CONNS_CLOSE_ERROR
                     , ACCEPT_LOOP_POLLIN_ERROR
                     , NETWORK_ERROR
                     , INTERNAL_ERROR
                     , ACCEPT_LOOP_CHKLOCKERS_ERROR2
                     , NEGATIVE_NUMBER_OF_LOCKERS_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_locker_array_t lockers;
    
    FLOM_TRACE(("flom_accept_loop\n"));
    TRY {
        int loop = TRUE;

        flom_locker_array_init(&lockers);
        
        while (loop) {
            int ready_fd;
            guint i, n;
            struct pollfd *fds;
            gint number_of_lockers;
            
            if (FLOM_RC_OK != (ret_cod = flom_conns_clean(conns)))
                THROW(CONNS_CLEAN_ERROR);
            if (NULL == (fds = flom_conns_get_fds(conns)))
                THROW(CONNS_GET_FDS_ERROR);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            FLOM_TRACE(("flom_accept_loop: entering poll...\n"));
            ready_fd = poll(fds, flom_conns_get_used(conns),
                            global_config.idle_time);
            FLOM_TRACE(("flom_accept_loop: ready_fd=%d\n", ready_fd));
            /* error on poll function */
            if (0 > ready_fd)
                THROW(POLL_ERROR);
            /* poll exited due to time out */
            if (0 == ready_fd) {
                number_of_lockers = flom_locker_array_count(&lockers);
                FLOM_TRACE(("flom_accept_loop: idle time exceeded %d "
                            "milliseconds, number of lockers=%d\n",
                            global_config.idle_time, number_of_lockers));
                if (0 == number_of_lockers) {
                    if (1 == flom_conns_get_used(conns)) {
                        FLOM_TRACE(("flom_accept_loop: only listener "
                                    "connection is active, exiting...\n"));
                        loop = FALSE;
                    }
                } else if (0 < number_of_lockers) {
                    if (FLOM_RC_OK != (ret_cod =
                                       flom_accept_loop_chklockers(&lockers)))
                        THROW(ACCEPT_LOOP_CHKLOCKERS_ERROR1);
                } else if (0 > number_of_lockers) {
                    THROW(NEGATIVE_NUMBER_OF_LOCKERS_ERROR1);
                }
                continue;
            }
            /* scanning file descriptors */
            n = flom_conns_get_used(conns);
            for (i=0; i<n; ++i) {
                FLOM_TRACE(("flom_accept_loop: i=%u, fd=%d, POLLIN=%d, "
                            "POLLERR=%d, POLLHUP=%d, POLLNVAL=%d\n", i,
                            fds[i].fd,
                            fds[i].revents & POLLIN,
                            fds[i].revents & POLLERR,
                            fds[i].revents & POLLHUP,
                            fds[i].revents & POLLNVAL));
                if ((fds[i].revents & POLLHUP) && (0 != i)) {
                    FLOM_TRACE(("flom_accept_loop: client %u disconnected "
                                "before categorization!\n", i));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           conns, i)))
                        THROW(CONNS_CLOSE_ERROR);
                    /* this file descriptor is no more valid, continue to
                       next one */
                    continue;
                }
                if (fds[i].revents & POLLIN) {
                    int conn_moved = FALSE;
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_pollin(
                                           conns, i, &lockers, &conn_moved)))
                        THROW(ACCEPT_LOOP_POLLIN_ERROR);
                    if (conn_moved)
                        /* connection is no more available for main thread,
                           a new main loop must be started */
                        break;
                }
                if (fds[i].revents &
                    (POLLERR | POLLHUP | POLLNVAL))
                    THROW(NETWORK_ERROR);
            } /* for (i... */
            /* check if any locker is ready for termination... */
            number_of_lockers = flom_locker_array_count(&lockers);
            FLOM_TRACE(("flom_accept_loop: number of lockers=%d\n",
                        number_of_lockers));
            if (0 < number_of_lockers) {
                if (FLOM_RC_OK != (ret_cod =
                                   flom_accept_loop_chklockers(&lockers)))
                    THROW(ACCEPT_LOOP_CHKLOCKERS_ERROR2);
            } else if (0 > number_of_lockers) {
                THROW(NEGATIVE_NUMBER_OF_LOCKERS_ERROR2);
            }
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_CLEAN_ERROR:
                break;
            case CONNS_GET_FDS_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_SET_EVENTS_ERROR:
                break;
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case ACCEPT_LOOP_CHKLOCKERS_ERROR1:
                break;
            case NEGATIVE_NUMBER_OF_LOCKERS_ERROR1:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case CONNS_CLOSE_ERROR:
            case ACCEPT_LOOP_POLLIN_ERROR:
                break;
            case NETWORK_ERROR:
                ret_cod = FLOM_RC_NETWORK_EVENT_ERROR;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case ACCEPT_LOOP_CHKLOCKERS_ERROR2:
                break;
            case NEGATIVE_NUMBER_OF_LOCKERS_ERROR2:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    flom_locker_array_free(&lockers);
    FLOM_TRACE(("flom_accept_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_pollin(flom_conns_t *conns, guint id,
                            flom_locker_array_t *lockers,
                            int *moved)
{
    enum Exception { CONNS_GET_CD_ERROR
                     , ACCEPT_ERROR
                     , CONNS_ADD_ERROR
                     , MSG_RETRIEVE_ERROR
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR
                     , PROTOCOL_ERROR
                     , ACCEPT_LOOP_TRANSFER_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_pollin\n"));
    TRY {
        struct flom_conn_data_s *c;

        *moved = FALSE;
        if (NULL == (c = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        FLOM_TRACE(("flom_accept_loop_pollin: id=%u, fd=%d\n",
                    id, c->fd));
        if (0 == id) {
            /* it's a new connection */
            int conn_fd;
            struct sockaddr cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            if (-1 == (conn_fd = accept(c->fd, &cliaddr, &clilen)))
                THROW(ACCEPT_ERROR);
            FLOM_TRACE(("flom_accept_loop_pollin: new client connected "
                        "with fd=%d\n", conn_fd));
            if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                                   conns, conn_fd, clilen, &cliaddr, TRUE)))
                THROW(CONNS_ADD_ERROR);
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            ssize_t read_bytes;
            struct flom_msg_s *msg;
            GMarkupParseContext *gmpc;
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                                   c->fd, buffer, sizeof(buffer),
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
                FLOM_TRACE(("flom_accept_loop_pollin: message from client %u "
                            "is invalid, disconneting...\n", id));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR);            
            }
            /* check if the message is completely parsed and can be transferred
               to a slave thread (a locker) */
            if (FLOM_MSG_STATE_READY == msg->state) {
                /* check the message is protocol correct */
                if (!flom_msg_check_protocol(msg, TRUE))
                    THROW(PROTOCOL_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_accept_loop_transfer(
                                       conns, id, lockers)))
                    THROW(ACCEPT_LOOP_TRANSFER_ERROR);
                *moved = TRUE;
            } /* if (FLOM_MSG_STATE_READY == msg->state) */
        } /* if (0 == id) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_GET_CD_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
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
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
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



int flom_accept_loop_transfer(flom_conns_t *conns, guint id,
                              flom_locker_array_t *lockers)
{
    enum Exception { NULL_OBJECT1
                     , INVALID_VERB_STEP
                     , LOCKER_CHECK_RESOURCE_NAME_ERROR
                     , NULL_OBJECT2
                     , CONNS_GET_MSG_ERROR
                     , RESOURCE_INIT_ERROR
                     , PIPE_ERROR
                     , G_THREAD_CREATE_ERROR
                     , WRITE_ERROR1
                     , CONNS_GET_CD_ERROR
                     , CONNS_TRNS_FD
                     , WRITE_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_locker_s *locker = NULL;
    int locker_allocated = FALSE;
    
    FLOM_TRACE(("flom_accept_loop_transfer\n"));
    TRY {
        gint i, n;
        int found = FALSE;
        GThread *locker_thread = NULL;
        struct flom_msg_s *msg = NULL;
        struct flom_locker_token_s flt;
        flom_rsrc_type_t flrt;
        const struct flom_conn_data_s *cd = NULL;
        /* check if there is a locker running for this request */
        if (NULL == lockers)
            THROW(NULL_OBJECT1);
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
        /* is the asked resource a valid resource name (and type!) ? */
        if (FLOM_RSRC_TYPE_NULL == (
                flrt = flom_rsrc_get_type(
                    msg->body.lock_8.resource.name)))
            /* @@@ send and error message to client and disconnect instead of
             exiting! */
            THROW(LOCKER_CHECK_RESOURCE_NAME_ERROR);
                                              
        /* is there a locker already active? */
        n = flom_locker_array_count(lockers);
        for (i=0; i<n; ++i) {
            if (NULL == (locker = flom_locker_array_get(lockers, i)))
                THROW(NULL_OBJECT2);
            if (NULL_FD == locker->write_pipe ||
                NULL_FD == locker->read_pipe) {
                FLOM_TRACE(("flom_accept_loop_transfer: locker # %d is "
                            "terminating (write_pipe=%d, read_pipe=%d), "
                            "skipping...\n", i, locker->write_pipe,
                            locker->read_pipe));
                continue;
            }
            FLOM_TRACE(("flom_accept_loop_transfer: locker # %d is managing "
                        "resource '%s'\n", i,
                        flom_resource_get_name(&locker->resource)));
            if (!g_strcmp0(flom_resource_get_name(&locker->resource),
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
            if (FLOM_RC_OK != (ret_cod = flom_resource_init(
                                   &locker->resource, flrt,
                                   msg->body.lock_8.resource.name)))
                THROW(RESOURCE_INIT_ERROR);
            /* creating a communication pipe for the new thread */
            if (0 != pipe(pipefd))
                THROW(PIPE_ERROR);
            locker->read_pipe = pipefd[0];
            locker->write_pipe = pipefd[1];
            locker_thread = g_thread_create(flom_locker_loop, (gpointer)locker,
                                            TRUE, &error_thread);
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

        /* prepare the token for locker thread */
        flt.domain = flom_conns_get_domain(conns);
        flt.client_fd = flom_conns_get_fd(conns, id);
        flt.sequence = ++locker->write_sequence;
        FLOM_TRACE(("flom_accept_loop_transfer: transferring connection %u "
                    "(domain=%d, client_fd=%d, sequence=%d) to thread %p "
                    "using pipe %d\n", id, flt.domain, flt.client_fd,
                    flt.sequence, locker_thread, locker->write_pipe));
        /* send token */
        if (sizeof(flt) != write(
                locker->write_pipe, &flt, sizeof(flt)))
            THROW(WRITE_ERROR1);
        /* send connection data (pointer is used because this object will
           be managed by child thread */
        if (NULL == (cd = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        /* set the connection as transferred to another thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_trns_fd(conns, id)))
            THROW(CONNS_TRNS_FD);
        if (sizeof(cd) != write(locker->write_pipe, &cd, sizeof(cd)))
            THROW(WRITE_ERROR2);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INVALID_VERB_STEP:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case LOCKER_CHECK_RESOURCE_NAME_ERROR:
                break;
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_GET_MSG_ERROR:
            case RESOURCE_INIT_ERROR:
                break;
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case G_THREAD_CREATE_ERROR:
                ret_cod = FLOM_RC_G_THREAD_CREATE_ERROR;
                break;
            case WRITE_ERROR1:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case CONNS_GET_CD_ERROR:
            case CONNS_TRNS_FD:
                break;
            case WRITE_ERROR2:
                ret_cod = FLOM_RC_WRITE_ERROR;
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
            flom_resource_free(&locker->resource);
            g_free(locker);
        }
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_transfer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_chklockers(flom_locker_array_t *lockers)
{
    enum Exception { NULL_LOCKER
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_chklockers\n"));
    TRY {
        gint i;
        gint number_of_lockers = flom_locker_array_count(lockers);
        
        for (i=0; i<number_of_lockers; ++i) {
            struct flom_locker_s *fl = flom_locker_array_get(lockers, i);
            if (NULL == fl)
                THROW(NULL_LOCKER);
            if (fl->write_sequence == fl->read_sequence &&
                fl->idle_periods > 1) {
                if (fl->write_pipe != NULL_FD) {
                    FLOM_TRACE(("flom_accept_loop_chklockers: starting "
                                "termination for locker %i (thread=%p, "
                                "write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    if (-1 == close(fl->write_pipe))
                        THROW(CLOSE_ERROR);
                    fl->write_pipe = NULL_FD;
                } else if (fl->write_pipe == NULL_FD &&
                           fl->read_pipe == NULL_FD) {
                    gpointer thread_ret_cod;
                    FLOM_TRACE(("flom_accept_loop_chklockers: completing "
                                "termination for locker %i (thread=%p, "
                                "write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    /* this is a possibly locking function... :(
                       but it's necessary to release glib stuff...
                       in case of issues, move to POSIX standard thread... */
                    thread_ret_cod = g_thread_join(fl->thread);
                    FLOM_TRACE(("flom_accept_loop_chklockers/g_thread_join"
                                "(%p)=%p\n", fl->thread, thread_ret_cod));
                    flom_locker_array_del(lockers, fl);
                    /* lockers object changed, break the loop */
                    break;
                }
            } /* if (fl->write_sequence == fl->read_sequence && ... */
        } /* for (i=0; i<number_of_lockers; ++i) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_LOCKER:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CLOSE_ERROR:
                ret_cod = FLOM_RC_CLOSE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_chklockers/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

