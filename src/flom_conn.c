/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif



#include "flom_conn.h"
#include "flom_config.h"
#include "flom_errors.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNS



flom_conn_t *flom_conn_new(flom_config_t *config)
{
    flom_conn_t *tmp;
    /* allocate the object itself */
    if (NULL != (tmp = g_try_malloc0(sizeof(flom_conn_t)))) {
        /* allocate msg struct */
        if (NULL != (tmp->msg = g_try_malloc(sizeof(struct flom_msg_s)))) {
            /* initialize the message */
            flom_msg_init(tmp->msg);
        } else {
            /* remove connection object because msg struct was not allocated */
            g_free(tmp);
            tmp = NULL;
        }
    } /* if (NULL != (tmp = g_try_malloc0(sizeof(flom_conn_t)))) */
    flom_tcp_init(&tmp->tcp, config);
    FLOM_TRACE(("flom_conn_new: obj=%p\n", tmp));
    return tmp;
}

    

void flom_conn_delete(flom_conn_t *obj)
{
    if (NULL != obj) {
        /* remove msg struct */
        if (NULL != obj->msg) {
            flom_msg_free(obj->msg);
            g_free(obj->msg);
            obj->msg = NULL;
        }
        /* clean TLS object */
        flom_tls_delete(obj->tls);
        obj->tls = NULL;
        /* remove object itself */
        g_free(obj);
    }
}



void flom_conn_free_parser(flom_conn_t *obj)
{
    if (NULL != obj) {
        if (NULL != obj->parser) {
            g_markup_parse_context_free(obj->parser);
            obj->parser = NULL;
        } /* if (NULL != obj->parser) */
    } /* if (NULL != obj) */
}



int flom_conn_send(flom_conn_t *obj, const void *buf, size_t len)
{
    int ret_cod = FLOM_RC_OK;
    FLOM_TRACE(("flom_conn_send\n"));

    if (NULL != obj->tls)
        ret_cod = flom_tls_send(obj->tls, buf, len);
    /* @@@ implement me
    else
        ret_cod = flom_tcp_send(&obj->tcp, buf, len);
    */
    FLOM_TRACE(("flom_conn_send/"
                "ret_cod=%d/errno=%d\n", ret_cod, errno));
    return ret_cod;
}



int flom_conn_recv(flom_conn_t *obj, void *buf, size_t len, size_t *received)
{
    int ret_cod = FLOM_RC_OK;
    FLOM_TRACE(("flom_conn_recv\n"));

    if (NULL != obj->tls)
        ret_cod = flom_tls_recv(obj->tls, buf, len, received);
    /* @@@ implement me
    else
        ret_cod = flom_tcp_send(&obj->tcp, buf, len, received);
    */
    FLOM_TRACE(("flom_conn_recv/"
                "ret_cod=%d/errno=%d\n", ret_cod, errno));
    return ret_cod;
}


void flom_conn_trace(const flom_conn_t *conn)
{
    FLOM_TRACE(("flom_conn_trace: object=%p\n", conn));
    FLOM_TRACE(("flom_conn_trace: "
                "fd=%d, type=%d, state=%d, wait=%d, msg=%p, parser=%p, "
                "addr_len=%d\n",
                flom_tcp_get_sockfd(&conn->tcp),
                flom_tcp_get_socket_type(&conn->tcp),
                conn->state, conn->wait, conn->msg, conn->parser,
                flom_tcp_get_addrlen(&conn->tcp)));
}



int flom_conn_set_keepalive(flom_config_t *config, int fd)
{
    enum Exception { NULL_OBJECT
                     , SETSOCKOPT_ERROR1
                     , SETSOCKOPT_ERROR2
                     , SETSOCKOPT_ERROR3
                     , SETSOCKOPT_ERROR4
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conn_set_keepalive\n"));
    TRY {
        int optval;
        socklen_t optlen = sizeof(optval);
        
        if (FLOM_NULL_FD == fd)
            THROW(NULL_OBJECT);

        FLOM_TRACE(("flom_conn_set_keepalive: setting SO_KEEPALIVE "
                    "for socket fd=%d\n", fd));
        /* set SO_KEEPALIVE feature for this socket */
        optval = 1;
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen))
            THROW(SETSOCKOPT_ERROR1);
        /* set tcp_keepalive_time parameter related to SO_KEEPALIVE */
        optval = flom_config_get_tcp_keepalive_time(config);
        if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen))
            THROW(SETSOCKOPT_ERROR2);
        /* set tcp_keepalive_intvl parameter related to SO_KEEPALIVE */
        optval = flom_config_get_tcp_keepalive_intvl(config);
        if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen))
            THROW(SETSOCKOPT_ERROR3);
        /* set tcp_keepalive_probes parameter related to SO_KEEPALIVE */
        optval = flom_config_get_tcp_keepalive_probes(config);
        if (-1 == setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &optval, optlen))
            THROW(SETSOCKOPT_ERROR4);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case SETSOCKOPT_ERROR1:
            case SETSOCKOPT_ERROR2:
            case SETSOCKOPT_ERROR3:
            case SETSOCKOPT_ERROR4:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conn_set_keepalive/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

