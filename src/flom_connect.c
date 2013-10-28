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



int flom_connect(const flom_config_t *config)
{
    enum Exception { SOCKET_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED
                     , CONNECT_LOCK_ERROR
                     , CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect\n"));
    TRY {
        int sockfd;
        struct sockaddr_un servaddr;
        
        FLOM_TRACE(("flom_connect: connecting to socket '%s'\n",
                    config->local_socket_path_name));

        if (-1 == (sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = AF_LOCAL;
        strcpy(servaddr.sun_path, config->local_socket_path_name);
        if (-1 == connect(sockfd, (struct sockaddr *)&servaddr,
                          sizeof(servaddr))) {
            if (ENOENT == errno || ECONNREFUSED == errno) {
                FLOM_TRACE(("flom_connect: connection failed, activating "
                            "a new daemon\n"));
                /* daemon is not active, starting it... */
                if (FLOM_RC_OK != (ret_cod = flom_daemon(config)))
                    THROW(DAEMON_ERROR);
                /* trying to connect again... */
                if (-1 == connect(sockfd, (struct sockaddr *)&servaddr,
                                  sizeof(servaddr)))
                    THROW(DAEMON_NOT_STARTED);
                FLOM_TRACE(("flom_connect: connected to flom daemon\n"));
                /* sending lock command */
                if (FLOM_RC_OK != (ret_cod = flom_connect_lock(config, sockfd)))
                    THROW(CONNECT_LOCK_ERROR);
            } else {
                THROW(CONNECT_ERROR);
            }
        }
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



int flom_connect_lock(const flom_config_t *config, int fd)
{
    enum Exception { G_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_connect_lock\n"));
    TRY {
        struct flom_msg_s msg;
        char buffer[1024];
        size_t to_send;

        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_LOCK;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;

        if (NULL == (msg.body.lock_8.resource.name =
                     g_strdup(config->resource_name)))
            THROW(G_STRDUP_ERROR);
        msg.body.lock_8.resource.type = FLOM_MSG_LOCK_TYPE_EX;
        msg.body.lock_8.resource.wait = TRUE;

        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        if (FLOM_RC_OK != (ret_cod = flom_msg_send(
                               fd, buffer, to_send)))
            THROW(MSG_SEND_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_connect_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

