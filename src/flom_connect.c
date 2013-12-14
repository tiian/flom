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



#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_connect.h"
#include "flom_daemon.h"
#include "flom_errors.h"
#include "flom_msg.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNECT



int flom_connect()
{
    enum Exception { SOCKET_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED
                     , CONNECT_ERROR
                     , CONNECT_LOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect\n"));
    TRY {
        struct flom_conn_data_s cd;

        memset(&cd, 0, sizeof(cd));
        
        FLOM_TRACE(("flom_connect: connecting to socket '%s'\n",
                    global_config.local_socket_path_name));

        if (-1 == (cd.fd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        cd.saun.sun_family = AF_LOCAL;
        strcpy(cd.saun.sun_path, global_config.local_socket_path_name);
        cd.addr_len = sizeof(cd.saun);
        if (-1 == connect(cd.fd, (struct sockaddr *)&cd.saun,
                          cd.addr_len)) {
            if (ENOENT == errno || ECONNREFUSED == errno) {
                FLOM_TRACE(("flom_connect: connection failed, activating "
                            "a new daemon\n"));
                /* daemon is not active, starting it... */
                if (FLOM_RC_OK != (ret_cod = flom_daemon()))
                    THROW(DAEMON_ERROR);
                /* trying to connect again... */
                if (-1 == connect(cd.fd, (struct sockaddr *)&cd.saun,
                                  cd.addr_len))
                    THROW(DAEMON_NOT_STARTED);
                FLOM_TRACE(("flom_connect: connected to flom daemon\n"));
            } else {
                THROW(CONNECT_ERROR);
            }
        }
        /* sending lock command */
        if (FLOM_RC_OK != (ret_cod = flom_connect_lock(&cd)))
            THROW(CONNECT_LOCK_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case DAEMON_ERROR:
                break;
            case DAEMON_NOT_STARTED:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case CONNECT_LOCK_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_connect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_connect_lock(struct flom_conn_data_s *cd)
{
    enum Exception { G_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , MSG_RETRIEVE_ERROR
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , PROTOCOL_ERROR1
                     , CONNECT_WAIT_LOCK_ERROR
                     , LOCK_BUSY
                     , PROTOCOL_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect_lock\n"));
    TRY {
        struct flom_msg_s msg;
        char buffer[1024];
        size_t to_send;
        ssize_t to_read;

        /* prepare a request (lock) message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_LOCK;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;

        if (NULL == (msg.body.lock_8.resource.name =
                     g_strdup(global_config.resource_name)))
            THROW(G_STRDUP_ERROR);
        msg.body.lock_8.resource.type = FLOM_LOCK_TYPE_EX;
        msg.body.lock_8.resource.wait = TRUE;

        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_send(
                               cd->fd, buffer, to_send)))
            THROW(MSG_SEND_ERROR);

        if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
            THROW(MSG_FREE_ERROR);
        flom_msg_init(&msg);

        /* retrieve the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                               cd->fd, buffer, sizeof(buffer), &to_read)))
            THROW(MSG_RETRIEVE_ERROR);

        /* instantiate a new parser */
        if (NULL == (cd->gmpc = g_markup_parse_context_new(
                         &flom_msg_parser, 0, (gpointer)&msg, NULL)))
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        
        /* deserialize the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                               buffer, to_read, &msg, cd->gmpc)))
            THROW(MSG_DESERIALIZE_ERROR);

        flom_msg_trace(&msg);

        /* check lock answer */
        if (FLOM_MSG_VERB_LOCK != msg.header.pvs.verb ||
            2*FLOM_MSG_STEP_INCR != msg.header.pvs.step)
            THROW(PROTOCOL_ERROR1);
        switch (msg.body.lock_16.answer.rc) {
            case FLOM_RC_OK:
                break;
            case FLOM_RC_LOCK_ENQUEUED:
                FLOM_TRACE(("flom_connect_lock: resource is busy, "
                            "waiting...\n"));
                if (FLOM_RC_OK != (ret_cod = flom_connect_wait_lock(cd, &msg)))
                    THROW(CONNECT_WAIT_LOCK_ERROR);
                break;
            case FLOM_RC_LOCK_BUSY:
                ret_cod = msg.body.lock_16.answer.rc;
                THROW(LOCK_BUSY);
                break;
            default:
                THROW(PROTOCOL_ERROR2);
                break;
        } /* switch (msg.body.lock_16.answer.rc) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
            case MSG_RETRIEVE_ERROR:
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case MSG_DESERIALIZE_ERROR:
                break;
            case PROTOCOL_ERROR1:
            case PROTOCOL_ERROR2:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case CONNECT_WAIT_LOCK_ERROR:
            case LOCK_BUSY:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */

    /* release markup parser */
    if (NULL != cd->gmpc) {
        g_markup_parse_context_free(cd->gmpc);
        cd->gmpc = NULL;
    }
    
    FLOM_TRACE(("flom_connect_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_connect_wait_lock(struct flom_conn_data_s *cd,
                           struct flom_msg_s *msg)
{
    enum Exception { MSG_RETRIEVE_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , PROTOCOL_ERROR1
                     , LOCK_CANT_LOCK
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect_wait_lock\n"));
    TRY {
        char buffer[1024];
        ssize_t to_read;
        
        /* retrieve the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                               cd->fd, buffer, sizeof(buffer), &to_read)))
            THROW(MSG_RETRIEVE_ERROR);

        flom_msg_free(msg);
        flom_msg_init(msg);
        
        /* deserialize the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                               buffer, to_read, msg, cd->gmpc)))
            THROW(MSG_DESERIALIZE_ERROR);

        flom_msg_trace(msg);
        
        /* check lock answer */
        if (FLOM_MSG_VERB_LOCK != msg->header.pvs.verb ||
            3*FLOM_MSG_STEP_INCR != msg->header.pvs.step)
            THROW(PROTOCOL_ERROR1);
        if (FLOM_RC_OK != msg->body.lock_24.answer.rc) {
            FLOM_TRACE(("flom_connect_wait_lock: lock can NOT be acquired, "
                        "leaving...\n"));
            THROW(LOCK_CANT_LOCK);
        }   
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:               
                break;
            case PROTOCOL_ERROR1:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case LOCK_CANT_LOCK:
                ret_cod = FLOM_RC_LOCK_CANT_LOCK;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_connect_wait_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

