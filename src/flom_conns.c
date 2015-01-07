/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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



#include "flom_conns.h"
#include "flom_config.h"
#include "flom_errors.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNS



int flom_conns_check_n(flom_conns_t *conns)
{
    /* this is a dirty hack because this struct is opaque and this operation
       should not be done :( */
    typedef struct _GPtrArrayPriv {
        gpointer *pdata;
        guint len;
        guint size;
    } GPtrArrayPriv;
    GPtrArrayPriv *p = (GPtrArrayPriv *)conns->array;
    
    FLOM_TRACE(("flom_conns_check_n: p->len=%u, p->size=%u, "
                "conns->array->len=%u\n", p->len, p->size, conns->array->len));
    return conns->array->len == p->len;
}



void flom_conns_init(flom_conns_t *conns, int domain)
{
    FLOM_TRACE(("flom_conns_init\n"));
    conns->poll_array = NULL;
    conns->domain = domain;
    conns->array = g_ptr_array_new();
    FLOM_TRACE(("flom_conns_init: allocated array:%p\n", conns->array));
}



int flom_conns_add(flom_conns_t *conns, int fd, int type,
                   socklen_t addr_len, const struct sockaddr *sa,
                   int main_thread)
{
    enum Exception { G_TRY_MALLOC_ERROR1
                     , INVALID_DOMAIN
                     , G_TRY_MALLOC_ERROR2
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_conn_data_s *tmp;
    
    FLOM_TRACE(("flom_conns_add\n"));
    TRY {
        if (NULL == (tmp = g_try_malloc0(sizeof(struct flom_conn_data_s))))
            THROW(G_TRY_MALLOC_ERROR1);
        FLOM_TRACE(("flom_conns_add: allocated a new connection (%p)\n", tmp));
        switch (conns->domain) {
            case AF_UNIX:
                tmp->saun = *((struct sockaddr_un *)sa);
                break;
            case AF_INET:
                tmp->sain = *((struct sockaddr_in *)sa);
                break;
            default:
                THROW(INVALID_DOMAIN);
        }
        tmp->fd = fd;
        assert(SOCK_STREAM == type || SOCK_DGRAM == type);
        tmp->type = type;
        tmp->state = main_thread ?
            FLOM_CONN_STATE_DAEMON : FLOM_CONN_STATE_LOCKER;
        tmp->wait = FALSE;
        tmp->addr_len = addr_len;
        /* reset the associated message */
        if (NULL == (tmp->msg =
                     g_try_malloc(sizeof(struct flom_msg_s))))
            THROW(G_TRY_MALLOC_ERROR2); 
        FLOM_TRACE(("flom_conns_add: allocated a new message (%p)\n",
                    tmp->msg));
        flom_msg_init(tmp->msg);
        
        /* initialize the associated parser */
        tmp->gmpc = g_markup_parse_context_new (
            &flom_msg_parser, 0, (gpointer)(tmp->msg), NULL);
        if (NULL == tmp->gmpc)
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        FLOM_TRACE(("flom_conns_add: allocated a new parser (%p)\n",
                    tmp->gmpc));
        g_ptr_array_add(conns->array, tmp);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR1:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case INVALID_DOMAIN:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
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
    FLOM_TRACE(("flom_conns_add: excp=%d\n", excp));
    if (NONE != excp) {
        if (G_TRY_MALLOC_ERROR2 < excp) /* release message */
            g_free(tmp->msg);
        if (G_TRY_MALLOC_ERROR1 < excp) /* release connection data */
            g_free(tmp);
    }
    FLOM_TRACE(("flom_conns_add/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_conns_import(flom_conns_t *conns, int fd,
                      struct flom_conn_data_s *cd)
{
    FLOM_TRACE(("flom_conns_import\n"));
    flom_conn_data_trace(cd);
    g_ptr_array_add(conns->array, cd);
}
    


struct pollfd *flom_conns_get_fds(flom_conns_t *conns)
{
    struct pollfd *tmp = NULL;
    guint i;
    
    FLOM_TRACE(("flom_conns_get_fds\n"));
    /* resize the previous poll array */
    if (NULL == (tmp = (struct pollfd *)realloc(
                     conns->poll_array,
                     (size_t)(conns->array->len*sizeof(struct pollfd))))) {
        conns->poll_array = tmp;
        return NULL;
    }    
    /* reset the array */
    memset(tmp, 0, (size_t)(conns->array->len*sizeof(struct pollfd)));
    /* fill the poll array with file descriptors */
    for (i=0; i<conns->array->len; ++i) {
        tmp[i].fd =
            ((struct flom_conn_data_s *)g_ptr_array_index(
                conns->array, i))->fd;
    }
    conns->poll_array = tmp;
    return conns->poll_array;
}




int flom_conns_set_events(flom_conns_t *conns, short events)
{
    enum Exception { OBJECT_CORRUPTED
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_set_events\n"));
    TRY {
        guint i;
        for (i=0; i<conns->array->len; ++i) {
            struct flom_conn_data_s *c =
                (struct flom_conn_data_s *)g_ptr_array_index(conns->array, i);
            if (NULL_FD != c->fd)
                conns->poll_array[i].events = events;
            else {
                FLOM_TRACE(("flom_conns_set_events: i=%u, "
                            "conns->poll_array[i].fd=%d\n", i,
                            conns->poll_array[i].fd));
                THROW(OBJECT_CORRUPTED);
            }
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OBJECT_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_set_events/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_close_fd(flom_conns_t *conns, guint id)
{
    enum Exception { OUT_OF_RANGE
                     , NULL_OBJECT
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_close_fd\n"));
    TRY {
        struct flom_conn_data_s *c;
        FLOM_TRACE(("flom_conns_close: closing connection id=%u\n", id));
        if (id >= conns->array->len)
            THROW(OUT_OF_RANGE);
        if (NULL == (c = (struct flom_conn_data_s *)
                     g_ptr_array_index(conns->array, id)))
            THROW(NULL_OBJECT);
        if (FLOM_CONN_STATE_REMOVE != c->state) {
            c->state = FLOM_CONN_STATE_REMOVE;
            if (NULL_FD == c->fd) {
                FLOM_TRACE(("flom_conns_close: connection id=%u already "
                            "closed, skipping...\n", id));
            } else {
                FLOM_TRACE(("flom_conns_close: closing fd=%d\n", c->fd));
                if (0 != close(c->fd))
                    THROW(CLOSE_ERROR);
                c->fd = NULL_FD;
            }
        } else {
            FLOM_TRACE(("flom_conns_close: connection id=%u already "
                        "in state %d, skipping...\n", id, c->state));
        } /* if (FLOM_CONN_STATE_REMOVE == c->state) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case NULL_OBJECT:
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
    FLOM_TRACE(("flom_conns_close_fd/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_trns_fd(flom_conns_t *conns, guint id)
{
    enum Exception { OUT_OF_RANGE
                     , G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_trns_fd\n"));
    TRY {
        struct flom_conn_data_s *c;
        FLOM_TRACE(("flom_conns_trns: marking as transferred connection "
                    "id=%u\n", id));
        if (id >= conns->array->len)
            THROW(OUT_OF_RANGE);
        /* update connection state */
        c = (struct flom_conn_data_s *)g_ptr_array_index(conns->array, id);
        c->state = FLOM_CONN_STATE_LOCKER;
        /* detach the connection from this connections object (it
           will be attached by a locker connections object */
        if (NULL == g_ptr_array_remove_index_fast(conns->array, id)) {
            THROW(G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR:
                ret_cod = FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_trns_fd/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_clean(flom_conns_t *conns)
{
    enum Exception { G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    guint i=0;
    
    FLOM_TRACE(("flom_conns_clean\n"));
    TRY {
        while (i<conns->array->len) {
            struct flom_conn_data_s *c =
                (struct flom_conn_data_s *)g_ptr_array_index(conns->array, i);
            FLOM_TRACE(("flom_conns_clean: i=%u, state=%d, wait=%d, "
                        "fd=%d %s\n",
                        i, c->state, c->wait, c->fd,
                        FLOM_CONN_STATE_REMOVE == c->state ?
                        "(removing...)" : FLOM_EMPTY_STRING));
            flom_conn_data_trace(c);
            if (FLOM_CONN_STATE_REMOVE == c->state) {
                /* connections with this state are no more valid and must be
                   removed and destroyed */
                /* removing message object */
                if (NULL != c->msg) {
                    flom_msg_free(c->msg);
                    g_free(c->msg);
                    c->msg = NULL;
                }
                /* removing parser object */
                if (NULL != c->gmpc) {
                    g_markup_parse_context_free(c->gmpc);
                    c->gmpc = NULL;
                }
                /* removing from array */
                if (NULL == g_ptr_array_remove_index_fast(conns->array, i)) {
                    THROW(G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR);
                }
                /* release connection */
                FLOM_TRACE(("flom_conns_clean: releasing connection %p\n", c));
                g_free(c);
            } else i++;
        } /* while (i<conns->n) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            case G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR:
                ret_cod = FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_conns_free(flom_conns_t *conns)
{
    guint i;
    FLOM_TRACE(("flom_conns_free: starting...\n"));
    for (i=0; i<conns->array->len; ++i)
        flom_conns_close_fd(conns, i);
    flom_conns_clean(conns);
    if (NULL != conns->poll_array) {
        FLOM_TRACE(("flom_conns_free: releasing poll_array:%p\n",
                    conns->poll_array));
        free(conns->poll_array);
        conns->poll_array = NULL;
    }
    assert(flom_conns_check_n(conns));
    if (NULL != conns->array) {
        FLOM_TRACE(("flom_conns_free: releasing array:%p\n",
                    conns->array));
        g_ptr_array_free(conns->array, TRUE);
        conns->array = NULL;
    }
    FLOM_TRACE(("flom_conns_free: completed\n"));
}



void flom_conn_data_trace(const struct flom_conn_data_s *conn)
{
    FLOM_TRACE(("flom_conn_data_trace: object=%p\n", conn));
    FLOM_TRACE(("flom_conn_data_trace: "
                "fd=%d, type=%d, state=%d, wait=%d, msg=%p, gmpc=%p, "
                "addr_len=%d\n",
                conn->fd, conn->type, conn->state, conn->wait, conn->msg,
                conn->gmpc, conn->addr_len));
}



void flom_conns_trace(const flom_conns_t *conns)
{
    FLOM_TRACE(("flom_conns_trace: object=%p\n", conns));
    FLOM_TRACE(("flom_conns_trace: domain=%d, len=%u, array=%p, "
                "poll_array=%p\n",
                conns->domain, conns->array->len, conns->array,
                conns->poll_array));
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
        
        if (NULL_FD == fd)
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

