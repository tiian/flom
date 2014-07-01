/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_REGEX_H
# include <regex.h>
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
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_LOCKER



void flom_locker_destroy(struct flom_locker_s *locker)
{
    if (NULL != locker) {
        flom_resource_free(&locker->resource);
        if (NULL_FD != locker->write_pipe)
            close(locker->write_pipe);
        if (NULL_FD != locker->read_pipe)
            close(locker->read_pipe);
        g_free(locker);
    }
}



void flom_locker_array_init(flom_locker_array_t *lockers)
{
    lockers->n = 0;
    lockers->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)flom_locker_destroy);
}



void flom_locker_array_free(flom_locker_array_t *lockers)
{
    g_ptr_array_free(lockers->array, TRUE);
    lockers->array = NULL;
    lockers->n = 0;
}



void flom_locker_array_add(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    g_ptr_array_add(lockers->array, (gpointer)locker);
    lockers->n++;
}



void flom_locker_array_del(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    if (g_ptr_array_remove(lockers->array, locker)) {
        FLOM_TRACE(("flom_locker_array_del: removed locker %p from array\n",
                    locker));
        lockers->n--;
    } else {
        FLOM_TRACE(("flom_locker_array_del: locker %p not found in array\n",
                    locker));
    }
}



gpointer flom_locker_loop(gpointer data)
{
    enum Exception { CONNS_CLEAN_ERROR
                     , CONNS_ADD_ERROR
                     , CONNS_GET_FDS_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , CONNS_CLOSE_ERROR1
                     , RESOURCE_CLEAN_ERROR1
                     , ACCEPT_LOOP_POLLIN_ERROR
                     , CONNS_CLOSE_ERROR2
                     , RESOURCE_CLEAN_ERROR2
                     , CONNS_CLOSE_ERROR3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_conns_t conns;
    
    FLOM_TRACE(("flom_locker_loop: new thread in progress (first message)\n"));
    TRY {
        int loop = TRUE;
        struct flom_locker_s *locker = (struct flom_locker_s *)data;
        struct flom_conn_data_s cd;

        /* as a first action, it marks the identifier */
        locker->thread = g_thread_self();
        FLOM_TRACE(("flom_locker_loop: resource_name='%s', "
                    "resource_type=%d\n",
                    flom_resource_get_name(&locker->resource),
                    flom_resource_get_type(&locker->resource)));
        /* initialize a connections object for this locker thread */
        flom_conns_init(&conns, AF_UNIX);
        /* add the parent communication pipe to connections */
        memset(&cd, 0, sizeof(cd));
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               &conns, locker->read_pipe, SOCK_STREAM,
                               sizeof(cd.sa), &(cd.sa), FALSE)))
            THROW(CONNS_ADD_ERROR);
        
        while (loop) {
            int ready_fd;
            guint i, n;
            struct pollfd *fds;
            int timeout = FLOM_LOCKER_POLL_TIMEOUT;
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
            /* set a specific timeout only if termination phase is not yet
               started and if specified lifespan is not null */
            if (locker->idle_periods == 0 &&
                locker->idle_lifespan > 0)
                timeout = locker->idle_lifespan;
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
                                           flom_conns_get_cd(&conns, i))))
                        THROW(RESOURCE_CLEAN_ERROR1);
                    /* conns is no more consistent, break the loop and poll
                       again */
                    break;
                } /* if (fds[i].revents & POLLERR) */
                if (fds[i].revents & POLLIN) {
                    if (FLOM_RC_OK != (ret_cod = flom_locker_loop_pollin(
                                           locker, &conns, i, &refresh_conns)))
                        THROW(ACCEPT_LOOP_POLLIN_ERROR);
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
                            THROW(CONNS_CLOSE_ERROR2);
                        /* clean locks and/or queued locks */
                        if (FLOM_RC_OK != (ret_cod =
                                           locker->resource.clean(
                                               &locker->resource,
                                               flom_conns_get_cd(&conns, i))))
                            THROW(RESOURCE_CLEAN_ERROR2);
                    } else {
                        /* locker termination asked by parent thread */
                        FLOM_TRACE(("flom_locker_loop: termination of this "
                                    "locker was asked by parent thread...\n"));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               &conns, i)))
                            THROW(CONNS_CLOSE_ERROR3);
                        locker->read_pipe = NULL_FD;
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
            case CONNS_CLEAN_ERROR:
            case CONNS_ADD_ERROR:
                break;
            case CONNS_GET_FDS_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_SET_EVENTS_ERROR:
            case POLL_ERROR:
            case CONNS_CLOSE_ERROR1:
            case RESOURCE_CLEAN_ERROR1:
            case ACCEPT_LOOP_POLLIN_ERROR:
            case CONNS_CLOSE_ERROR2:
            case RESOURCE_CLEAN_ERROR2:
            case CONNS_CLOSE_ERROR3:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
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
                            int *refresh_conns)
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
    
    struct flom_conn_data_s *new_cd = NULL;
    
    FLOM_TRACE(("flom_locker_loop_pollin\n"));
    TRY {
        *refresh_conns = FALSE;
        struct flom_msg_s *msg = NULL;
        struct flom_conn_data_s *curr_cd;
        
        if (NULL == (curr_cd = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d\n",
                    id, curr_cd->fd));
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
            if (sizeof(new_cd) != read(
                    locker->read_pipe, &new_cd, sizeof(new_cd)))
                THROW(READ_ERROR2);
            /* import the connection passed by parent thread */
            flom_conns_import(conns, flt.client_fd, new_cd);
            /* set the locker sequence */
            locker->read_sequence = flt.sequence;
            /* retrieve the message sent by the client */
            msg = new_cd->msg;
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            ssize_t read_bytes;
            GMarkupParseContext *gmpc;
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                                   curr_cd->fd, curr_cd->type,
                                   buffer, sizeof(buffer),
                                   &read_bytes, FLOM_NETWORK_WAIT_TIMEOUT,
                                   NULL, NULL)))
                THROW(MSG_RETRIEVE_ERROR);

            if (0 == read_bytes) {
                /* connection closed */
                FLOM_TRACE(("flom_locker_loop_pollin: id=%d, fd=%d "
                            "returned 0 bytes: disconnecting...\n",
                            id, curr_cd->fd));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR1);
                *refresh_conns = TRUE;
                /* clean lock state if any lock was acquired... */
                if (FLOM_RC_OK != (ret_cod = 
                                   locker->resource.clean(
                                       &locker->resource, curr_cd)))
                    THROW(RESOURCE_CLEAN_ERROR);
            } else {
                /* data arrived */
                if (NULL == (msg = flom_conns_get_msg(conns, id)))
                    THROW(CONNS_GET_MSG_ERROR);

                if (NULL == (gmpc = flom_conns_get_gmpc(conns, id)))
                    THROW(CONNS_GET_GMPC_ERROR);
            
                if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                       buffer, read_bytes, msg, gmpc)))
                    THROW(MSG_DESERIALIZE_ERROR);
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
            if (NULL != new_cd)
                curr_cd = new_cd;
            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb ||
                FLOM_MSG_VERB_UNLOCK == msg->header.pvs.verb) {
                /* process input message */
                if (FLOM_RC_OK != (ret_cod = 
                                   locker->resource.inmsg(
                                       &locker->resource, curr_cd, msg)))
                    THROW(RESOURCE_INMSG_ERROR);
                /* reply with output message */
                if (FLOM_MSG_STATE_READY == msg->state) {
                    char buffer[FLOM_MSG_BUFFER_SIZE];
                    size_t msg_len = 0;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                                           msg, buffer, sizeof(buffer),
                                           &msg_len)))
                        THROW(MSG_SERIALIZE_ERROR);
                    ret_cod = flom_msg_send(curr_cd->fd, buffer, msg_len);
                    if (FLOM_RC_SEND_ERROR == ret_cod) {
                        FLOM_TRACE(("flom_locker_loop_pollin: error while "
                                    "sending message to client (the "
                                    "connection) will be closed during next "
                                    "poll loop...\n"));
                    } else if (FLOM_RC_OK != ret_cod)
                        THROW(MSG_SEND_ERROR);
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

