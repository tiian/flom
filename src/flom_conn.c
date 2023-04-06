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
    FLOM_TRACE(("flom_conn_delete: obj=%p\n", obj));
    if (NULL != obj) {
        FLOM_TRACE(("flom_conn_delete: obj->msg=%p\n", obj->msg));
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



int flom_conn_init(flom_conn_t *obj, int domain, int sockfd, int type,
                   socklen_t addrlen, const struct sockaddr *sa,
                   int main_thread)
{
    enum Exception { NULL_OBJECT
                     , INVALID_DOMAIN
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conn_init\n"));
    TRY {
        GMarkupParseContext *tmp_parser = NULL;
        if (NULL == obj)
            THROW(NULL_OBJECT);
        
        flom_tcp_init(&obj->tcp, NULL);
        flom_tcp_set_domain(&obj->tcp, domain);
        /* set address */
        switch (domain) {
            case AF_UNIX:
                memcpy(flom_tcp_get_sa_un(&obj->tcp), sa,
                       sizeof(struct sockaddr_un));
                break;
            case AF_INET:
                memcpy(flom_tcp_get_sa_in(&obj->tcp), sa,
                       sizeof(struct sockaddr_in));
                break;
            case AF_INET6:
                memcpy(flom_tcp_get_sa_in6(&obj->tcp), sa,
                       sizeof(struct sockaddr_in6));
                break;
            default:
                THROW(INVALID_DOMAIN);
        }
        flom_tcp_set_addrlen(&obj->tcp, addrlen);
        flom_tcp_set_sockfd(&obj->tcp, sockfd);
        assert(SOCK_STREAM == type || SOCK_DGRAM == type);
        flom_tcp_set_socket_type(&obj->tcp, type);
        flom_conn_set_state(obj, main_thread ?
                            FLOM_CONN_STATE_DAEMON : FLOM_CONN_STATE_LOCKER);
        flom_conn_set_wait(obj, FALSE);
        
        /* initialize the associated parser */
        if (NULL == (tmp_parser = g_markup_parse_context_new (
                         &flom_msg_parser, 0,
                         (gpointer)(obj->msg), NULL)))
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        flom_conn_set_parser(obj, tmp_parser);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INVALID_DOMAIN:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conn_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
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
    else
        ret_cod = flom_tcp_send(&obj->tcp, buf, len);
    FLOM_TRACE(("flom_conn_send/"
                "ret_cod=%d/errno=%d\n", ret_cod, errno));
    return ret_cod;
}



int flom_conn_recv(flom_conn_t *obj, void *buf, size_t len, size_t *received,
                   int timeout, struct sockaddr *src_addr, socklen_t *addrlen)
{
    enum Exception { POLL_ERROR
                     , NETWORK_TIMEOUT
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conn_recv\n"));
    TRY {
        if (timeout > 0) {
            struct pollfd fds[1];
            int rc;
            /* use poll to check the filedescriptor for a limited amount of
               time */
            fds[0].fd = flom_tcp_get_sockfd(&obj->tcp);
            fds[0].events = POLLIN;
            fds[0].revents = 0;
            rc = poll(fds, 1, timeout);
            switch (rc) {
                case -1: /* error in poll function */
                    THROW(POLL_ERROR);
                    break;
                case 0: /* timeout, return! */
                    THROW(NETWORK_TIMEOUT);
                    break;
                case 1: /* data arrived, go on... */
                    break;
                default: /* unexpected result, internal error! */
                    THROW(INTERNAL_ERROR);
            } /* switch (rc) */
        } /* if (timeout > 0) */

        
        if (NULL != obj->tls)
            ret_cod = flom_tls_recv_msg(obj->tls, buf, len, received);
        else
            ret_cod = flom_tcp_recv(&obj->tcp, buf, len, received,
                                    src_addr, addrlen);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case NETWORK_TIMEOUT:
                ret_cod = FLOM_RC_NETWORK_TIMEOUT;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case NONE: /* return the ret_cod of the recv operation */
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conn_recv/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conn_terminate(flom_conn_t *obj)
{
    enum Exception { TCP_CLOSE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conn_terminate\n"));
    TRY {
        if (FLOM_CONN_STATE_REMOVE != flom_conn_get_state(obj)) {
            flom_conn_set_state(obj, FLOM_CONN_STATE_REMOVE);
            if (FLOM_NULL_FD == flom_tcp_get_sockfd(&obj->tcp)) {
                FLOM_TRACE(("flom_conn_terminate: connection %p already "
                            "closed, skipping...\n", obj));
            } else {
                FLOM_TRACE(("flom_conn_terminate: closing fd=%d\n",
                            flom_tcp_get_sockfd(&obj->tcp)));
                if (FLOM_RC_OK != (ret_cod = flom_tcp_close(&obj->tcp)))
                    THROW(TCP_CLOSE);
            }
        } else {
            FLOM_TRACE(("flom_conn_terminate: connection %p already "
                        "in state %d, skipping...\n", obj,
                        flom_conn_get_state(obj)));
        } /* if (FLOM_CONN_STATE_REMOVE == flom_conn_get_state(c)) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TCP_CLOSE:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conn_terminate/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
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



int flom_conn_authenticate(flom_conn_t *conn, const gchar *peer_id,
                           int tls_check_peer_id)
{
    enum Exception { TLS_CERT_CHECK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    gchar *unique_id = NULL;
    gchar *peer_addr = NULL;

    FLOM_TRACE(("flom_conn_authenticate\n"));
    TRY {
        if (NULL == flom_conn_get_tls(conn)) {
            FLOM_TRACE(("flom_conn_authenticate: this is not a TLS coonection "
                        "and authentication can not be performed\n"));
        } else if (tls_check_peer_id) {
            peer_addr = flom_tcp_retrieve_peer_name(
                flom_conn_get_tcp(conn));
            if (FLOM_RC_OK != (ret_cod = flom_tls_cert_check(
                                   flom_conn_get_tls(conn), peer_id,
                                   peer_addr))) {
                THROW(TLS_CERT_CHECK_ERROR);
            }            
        } /* if (tls_check_peer_id) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_CERT_CHECK_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unique id object clean-up */
    if (NULL != unique_id)
        g_free(unique_id);
    /* peer address object clean-up */
    if (NULL != peer_addr)
        g_free(peer_addr);
    FLOM_TRACE(("flom_conn_authenticate/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
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

