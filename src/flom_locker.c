/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_REGEX_H
# include <regex.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_locker.h"
#include "flom_rsrc.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_vfs.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_LOCKER



void flom_locker_destroy(struct flom_locker_s *locker)
{
    if (NULL != locker) {
        flom_resource_free(&locker->resource);
        if (FLOM_NULL_FD != locker->write_pipe)
            close(locker->write_pipe);
        if (FLOM_NULL_FD != locker->read_pipe)
            close(locker->read_pipe);
        g_free(locker);
    }
}



void flom_locker_array_init(flom_locker_array_t *lockers)
{
    lockers->locker_array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)flom_locker_destroy);
}



void flom_locker_array_free(flom_locker_array_t *lockers)
{
    g_ptr_array_free(lockers->locker_array, TRUE);
    lockers->locker_array = NULL;
}



void flom_locker_array_add(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    g_ptr_array_add(lockers->locker_array, (gpointer)locker);
    flom_vfs_ram_tree_add_locker(locker->uid, locker->resource.name,
                                 flom_rsrc_get_type_human_readable(
                                     locker->resource.type));
}



void flom_locker_array_del(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    flom_vfs_ram_tree_del_locker(locker->uid);
    if (g_ptr_array_remove(lockers->locker_array, locker)) {
        FLOM_TRACE(("flom_locker_array_del: removed locker %p from array\n",
                    locker));
    } else {
        FLOM_TRACE(("flom_locker_array_del: locker %p not found in array\n",
                    locker));
    }
}



gpointer flom_locker_loop(gpointer data)
{
    enum Exception { NEW_OBJ
                     , CONN_INIT_ERROR
                     , CONNS_CLEAN_ERROR
                     , CONNS_GET_FDS_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , RESOURCE_TIMEOUT_ERROR
                     , CONNS_CLOSE_ERROR1
                     , RESOURCE_CLEAN_ERROR1
                     , CONNS_CLOSE_ERROR2
                     , RESOURCE_CLEAN_ERROR2
                     , CONNS_CLOSE_ERROR3
                     , RESOURCE_CLEAN_ERROR3
                     , CONNS_CLOSE_ERROR4
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_conns_t conns;
    flom_conn_t *conn = NULL;
    
    FLOM_TRACE(("flom_locker_loop: new thread in progress (first message)\n"));
    TRY {
        int loop = TRUE;
        struct flom_locker_s *locker = (struct flom_locker_s *)data;
        struct sockaddr_storage sa_storage;
        struct timeval next_deadline;

        /* set next deadline to one second in the past*/
        gettimeofday(&next_deadline, NULL);
        next_deadline.tv_sec--;
        
        /* as a first action, it marks the identifier */
        locker->thread = g_thread_self();
        FLOM_TRACE(("flom_locker_loop: resource_name='%s', "
                    "resource_type=%d\n",
                    flom_resource_get_name(&locker->resource),
                    flom_resource_get_type(&locker->resource)));
        /* initialize a connections object for this locker thread */
        flom_conns_init(&conns, AF_UNIX);
        /* create a new connection object */
        if (NULL == (conn = flom_conn_new(NULL)))
            THROW(NEW_OBJ);
        FLOM_TRACE(("flom_locker_loop: allocated a new connection (%p)\n",
                    conn));
        /* initialize the connection */
        memset(&sa_storage, 0, sizeof(sa_storage));
        if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                               conn, 0,
                               flom_conns_get_domain(&conns),
                               locker->read_pipe, SOCK_STREAM,
                               sizeof(struct sockaddr_storage),
                               (struct sockaddr *)&sa_storage,
                               FALSE)))
            THROW(CONN_INIT_ERROR);
        
        /* add the parent communication pipe to connections */
        flom_conns_add_conn(&conns, conn);
        conn = NULL; /* avoid connection delete from this function */
        
        while (loop) {
            int ready_fd;
            guint i, n;
            struct pollfd *fds;
            int timeout;
            if (FLOM_RC_OK != (ret_cod = flom_conns_clean(&conns)))
                THROW(CONNS_CLEAN_ERROR);
            if (flom_conns_get_used(&conns) == 0) {
                FLOM_TRACE(("flom_locker_loop: no more available connections"
                            ", leaving...\n"));
                /* break the loop */
                loop = FALSE;
                break;
            }
            if (NULL == (fds = flom_conns_get_fds(&conns)))
                THROW(CONNS_GET_FDS_ERROR);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(
                                   &conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            FLOM_TRACE(("flom_locker_loop: "
                        "next_deadline.tv_sec=%d, next_deadline.tv_usec=%d\n",
                        next_deadline.tv_sec, next_deadline.tv_usec));
            /* compute time-out from next deadline as asked by the resource
               (in case the resource asked for a deadline...) */
            if (0 <= (timeout = flom_locker_loop_get_timeout(
                          &next_deadline))) {
                FLOM_TRACE(("flom_locker_loop: timeout was requested by the "
                            "resource: %d milliseconds\n", timeout));
            } else if (locker->idle_periods > FLOM_LOCKER_MAX_IDLE_PERIODS) {
                /* the only possible event comes from main thread, using
                   a shorter time-out would be useless */
                timeout = -1;
                FLOM_TRACE(("flom_locker_loop: locker termination already "
                            "started, using infinite poll timeout...\n"));
            } else if (locker->idle_periods > 0) {
                /* there's a chance this locker would start termination
                   because there are no connected clients */
                timeout = FLOM_LOCKER_POLL_TIMEOUT;
                FLOM_TRACE(("flom_locker_loop: possible locker termination "
                            "in the next few milliseconds, using a short "
                            "poll timeout...\n"));
            } else if (locker->idle_periods == 0 &&
                       locker->idle_lifespan > 0) {
                /* time-out must be fixed to resource lifespan as requested
                   by the client */
                timeout = locker->idle_lifespan;
                FLOM_TRACE(("flom_locker_loop: resource timeout asked by "
                            "caller (%d milliseconds)\n", timeout));
            } else {
                timeout = FLOM_LOCKER_POLL_TIMEOUT;
                FLOM_TRACE(("flom_locker_loop: setting default timeout: "
                            "%d milliseconds\n", timeout));
            }
            FLOM_TRACE(("flom_locker_loop: entering poll using %d "
                        "timeout milliseconds...\n", timeout));
            ready_fd = poll(fds, flom_conns_get_used(&conns), timeout);
            FLOM_TRACE(("flom_locker_loop: ready_fd=%d\n", ready_fd));
            /* error on poll function */
            if (0 > ready_fd)
                THROW(POLL_ERROR);
            /* poll exited due to time out */
            if (0 == ready_fd) {
                FLOM_TRACE(("flom_locker_loop: idle time exceeded %d "
                            "milliseconds\n", timeout));
                /* calling timeout resource callback */
                if (FLOM_RC_OK != (ret_cod = locker->resource.timeout(
                                       &locker->resource, &next_deadline)))
                    THROW(RESOURCE_TIMEOUT_ERROR);
                if (1 == flom_conns_get_used(&conns)) {
                    locker->idle_periods++;
                    FLOM_TRACE(("flom_locker_loop: only control connection "
                                "is active, idle_periods=%d, waiting exit "
                                "command from parent thread...\n",
                                locker->idle_periods));
                }
                continue;
            }
            /* scanning file descriptors */
            n = flom_conns_get_used(&conns);
            for (i=0; i<n; ++i) {
                int refresh_conns = FALSE;
                FLOM_TRACE(("flom_locker_loop: i=%u, fd=%d, POLLIN=%d, "
                            "POLLERR=%d, POLLHUP=%d, POLLNVAL=%d\n", i,
                            fds[i].fd,
                            fds[i].revents & POLLIN,
                            fds[i].revents & POLLERR,
                            fds[i].revents & POLLHUP,
                            fds[i].revents & POLLNVAL));
                if (fds[i].revents & POLLERR) {
                    /* client error, termination */
                    FLOM_TRACE(("flom_locker_loop: connection to client %u "
                                "encountered an error, closing it...\n", i));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           &conns, i)))
                        THROW(CONNS_CLOSE_ERROR1);
                    /* clean locks and/or queued locks */
                    if (FLOM_RC_OK != (ret_cod =
                                       locker->resource.clean(
                                           &locker->resource,
                                           flom_conns_get_conn(&conns, i))))
                        THROW(RESOURCE_CLEAN_ERROR1);
                    /* conns is no more consistent, break the loop and poll
                       again */
                    break;
                } /* if (fds[i].revents & POLLERR) */
                if (fds[i].revents & POLLIN) {
                    if (FLOM_RC_OK != (ret_cod = flom_locker_loop_pollin(
                                           locker, &conns, i,
                                           &refresh_conns, &next_deadline))) {
                        FLOM_TRACE(("flom_locker_loop: connection %u "
                                    "raised an exception, closing it...\n",
                                    i));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR2);
                        /* clean locks and/or queued locks */
                        if (FLOM_RC_OK != (ret_cod =
                                           locker->resource.clean(
                                               &locker->resource,
                                               flom_conns_get_conn(
                                                   &conns, i))))
                            THROW(RESOURCE_CLEAN_ERROR2);
                        /* conns is no more consistent, break the loop and poll
                           again */
                        refresh_conns = TRUE;
                    }
                    if (refresh_conns)
                        /* conns is no more consistent, break the loop and poll
                           again */
                        break;
                }
                if (fds[i].revents & POLLHUP) {
                    if (0 != i) {
                        /* client termination */
                        FLOM_TRACE(("flom_locker_loop: client %u "
                                    "disconnected\n", i));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR3);
                        /* clean locks and/or queued locks */
                        if (FLOM_RC_OK != (ret_cod =
                                           locker->resource.clean(
                                               &locker->resource,
                                               flom_conns_get_conn(
                                                   &conns, i))))
                            THROW(RESOURCE_CLEAN_ERROR3);
                    } else {
                        /* locker termination asked by parent thread */
                        FLOM_TRACE(("flom_locker_loop: termination of this "
                                    "locker was asked by parent thread...\n"));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR4);
                        locker->read_pipe = FLOM_NULL_FD;
                    }
                    /* conns is no more consistent, break the loop and poll
                       again */
                    break;
                } /* if (fds[i].revents & POLLHUP) */
            } /* for (i... */
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
            case CONNS_CLEAN_ERROR:
                break;
            case CONNS_GET_FDS_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_SET_EVENTS_ERROR:
            case POLL_ERROR:
            case RESOURCE_TIMEOUT_ERROR:
            case CONNS_CLOSE_ERROR1:
            case RESOURCE_CLEAN_ERROR1:
            case CONNS_CLOSE_ERROR2:
            case RESOURCE_CLEAN_ERROR2:
            case CONNS_CLOSE_ERROR3:
            case RESOURCE_CLEAN_ERROR3:
            case CONNS_CLOSE_ERROR4:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    /* clean-up connections object */
    flom_conns_free(&conns);
    FLOM_TRACE(("flom_locker_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    FLOM_TRACE(("flom_locker_loop: this thread completed service (last "
                "message)\n"));
    return data;
}



int flom_locker_loop_pollin(struct flom_locker_s *locker,
                            flom_conns_t *conns, guint id,
                            int *refresh_conns, struct timeval *next_deadline)
{
    enum Exception { CONNS_GET_CD_ERROR
                     , READ_ERROR1
                     , READ_ERROR2
                     , MSG_RETRIEVE_ERROR
                     , CONNS_CLOSE_ERROR1
                     , RESOURCE_CLEAN_ERROR
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR2
                     , RESOURCE_INMSG_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    flom_conn_t *new_conn = NULL;
    
    FLOM_TRACE(("flom_locker_loop_pollin\n"));
    TRY {
        *refresh_conns = FALSE;
        struct flom_msg_s *msg = NULL;
        flom_conn_t *curr_conn;
        
        if (NULL == (curr_conn = flom_conns_get_conn(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d\n",
                    id, flom_tcp_get_sockfd(flom_conn_get_tcp(curr_conn))));
        locker->idle_periods = 0;
        if (0 == id) {
            struct flom_locker_token_s flt;
            /* it's a connection passed by parent thread */
            /* pick-up token from parent thread */
            if (sizeof(flt) != read(
                    locker->read_pipe, &flt, sizeof(flt)))
                THROW(READ_ERROR1);
            flom_conns_set_domain(conns, flt.domain);
            FLOM_TRACE(("flom_locker_loop_pollin: receiving connection "
                    "(domain=%d, client_fd=%d, sequence=%d) using pipe %d\n",
                        flt.domain, flt.client_fd, flt.sequence,
                        locker->read_pipe));
            /* pick-up connection data pointer parent thread */
            if (sizeof(new_conn) != read(
                    locker->read_pipe, &new_conn, sizeof(new_conn)))
                THROW(READ_ERROR2);
            /* import the connection passed by parent thread */
            flom_conns_import(conns, flt.client_fd, new_conn);
            /* set the locker sequence */
            locker->read_sequence = flt.sequence;
            /* retrieve the message sent by the client */
            msg = flom_conn_get_msg(new_conn);
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            size_t read_bytes;
            GMarkupParseContext *gmpc;
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_conn_recv(
                                   curr_conn, buffer, sizeof(buffer),
                                   &read_bytes, FLOM_NETWORK_WAIT_TIMEOUT,
                                   NULL, NULL)))
                THROW(MSG_RETRIEVE_ERROR);

            if (0 == read_bytes) {
                /* connection closed */
                FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d "
                            "returned 0 bytes: disconnecting...\n",
                            id, flom_tcp_get_sockfd(
                                flom_conn_get_tcp(curr_conn))));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR1);
                *refresh_conns = TRUE;
                /* clean lock state if any lock was acquired... */
                if (FLOM_RC_OK != (ret_cod = 
                                   locker->resource.clean(
                                       &locker->resource, curr_conn)))
                    THROW(RESOURCE_CLEAN_ERROR);
            } else {
                /* data arrived */
                if (NULL == (msg = flom_conns_get_msg(conns, id)))
                    THROW(CONNS_GET_MSG_ERROR);

                if (NULL == (gmpc = flom_conns_get_parser(conns, id)))
                    THROW(CONNS_GET_GMPC_ERROR);
            
                if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                       buffer, read_bytes, msg, gmpc)))
                    THROW(MSG_DESERIALIZE_ERROR);
                flom_conn_set_last_step(curr_conn, msg->header.pvs.step);
                /* if the message is not valid the client must be terminated */
                if (FLOM_MSG_STATE_INVALID == msg->state) {
                    FLOM_TRACE(("flom_locker_loop_pollin: message from client "
                                "%i is invalid, disconneting...\n", id));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           conns, id)))
                        THROW(CONNS_CLOSE_ERROR2);
                }
            } /* if (0 == read_bytes) */
        } /* if (0 == id) */
        
        if (NULL != msg) {
            if (NULL != new_conn)
                curr_conn = new_conn;
            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb ||
                FLOM_MSG_VERB_UNLOCK == msg->header.pvs.verb) {
                /* process input message */
                if (FLOM_RC_OK != (ret_cod = 
                                   locker->resource.inmsg(
                                       &locker->resource, locker->uid,
                                       curr_conn, msg, next_deadline)))
                    THROW(RESOURCE_INMSG_ERROR);
                /* reply with output message */
                if (FLOM_MSG_STATE_READY == msg->state) {
                    char buffer[FLOM_MSG_BUFFER_SIZE];
                    size_t msg_len = 0;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                                           msg, buffer, sizeof(buffer),
                                           &msg_len)))
                        THROW(MSG_SERIALIZE_ERROR);
                    ret_cod = flom_conn_send(curr_conn, buffer, msg_len);
                    if (FLOM_RC_SEND_ERROR == ret_cod) {
                        FLOM_TRACE(("flom_locker_loop_pollin: error while "
                                    "sending message to client (the "
                                    "connection) will be closed during next "
                                    "poll loop...\n"));
                    } else if (FLOM_RC_OK != ret_cod)
                        THROW(MSG_SEND_ERROR);
                    flom_conn_set_last_step(curr_conn, msg->header.pvs.step);
                } /* if (FLOM_MSG_STATE_READY == msg->state) */
            } else {
                /* Implement ping message here... */
                FLOM_TRACE(("flom_locker_loop_pollin: unexpected message with "
                            "verb=%d was arrived!\n", msg->header.pvs.verb));
                THROW(PROTOCOL_ERROR);
            } /* if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb ... */
            /* free message content and reset it */
            if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                THROW(MSG_FREE_ERROR);
            flom_msg_init(msg);
        } /* if (NULL != msg) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_GET_CD_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case READ_ERROR1:
            case READ_ERROR2:
                ret_cod = FLOM_RC_READ_ERROR;
                break;
            case MSG_RETRIEVE_ERROR:
            case CONNS_CLOSE_ERROR1:
            case RESOURCE_CLEAN_ERROR:
                break;
            case CONNS_GET_MSG_ERROR:
            case CONNS_GET_GMPC_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case MSG_DESERIALIZE_ERROR:
            case CONNS_CLOSE_ERROR2:
            case RESOURCE_INMSG_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_locker_loop_pollin/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_locker_loop_get_timeout(const struct timeval *next_deadline)
{
    struct timeval now;
    int diff;

    gettimeofday(&now, NULL);
    diff = (next_deadline->tv_sec - now.tv_sec) * 1000 +
        (next_deadline->tv_usec - now.tv_usec) / 1000 + 1;
    FLOM_TRACE(("flom_locker_loop_get_timeout: "
                "next_deadline->tv_sec=%d, next_deadline->tv_usec=%d, "
                "now.tv_sec=%d, now.tv_usec=%d, diff=%d\n",
                next_deadline->tv_sec, next_deadline->tv_usec,
                now.tv_sec, now.tv_usec, diff));
    return diff;
}

