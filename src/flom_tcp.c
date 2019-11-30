/*
 * Copyright (c) 2013-2019, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
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



#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif


#include "flom_errors.h"
#include "flom_tcp.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_TCP



void flom_tcp_init(flom_tcp_t *obj, flom_config_t *config)
{
    FLOM_TRACE(("flom_tcp_init\n"));
    /* memory reset */
    memset(obj, 0, sizeof(flom_tcp_t));
    obj->config = config;
}



int flom_tcp_listen(flom_tcp_t *obj)
{
    enum Exception { GETADDRINFO_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    struct addrinfo *result = NULL;
    int fd = FLOM_NULL_FD;
    
    FLOM_TRACE(("flom_tcp_listen\n"));
    TRY {
        struct addrinfo hints, *gai = NULL;
        int errcode;
        char port[100];
        struct sockaddr_in6 sa6;
        struct sockaddr *sa = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = obj->domain;
        hints.ai_socktype = obj->socket_type;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_unicast_port(obj->config));
        FLOM_TRACE(("flom_tcp_listen: binding address '%s' "
                    "and port %s\n", flom_config_get_unicast_address(
                        obj->config), port));

        if (0 != (errcode = getaddrinfo(
                      flom_config_get_unicast_address(obj->config),
                      port, &hints, &result))) {
            FLOM_TRACE(("flom_tcp_listen/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            int bound = FALSE;
            int sock_opt = 1;
            FLOM_TRACE_ADDRINFO("flom_tcp_listen/getaddrinfo(): ",
                                result);
            /* traverse the list and try to bind... */
            gai = result;
            while (NULL != gai && !bound) {
                sa = gai->ai_addr;
                /* IPv6 addresses could need sin6_scope_id set if the user
                   specified a network interface */
                if (AF_INET6 == gai->ai_family &&
                    NULL != flom_config_get_network_interface(obj->config)) {
                    memcpy(&sa6, sa, gai->ai_addrlen);
                    sa6.sin6_scope_id = flom_config_get_sin6_scope_id(
                        obj->config);
                    sa = (struct sockaddr *)&sa6;
                    FLOM_TRACE(("flom_tcp_listen: overriding field "
                                "sin6_scope_id with value %u\n",
                                sa6.sin6_scope_id));
                }
                FLOM_TRACE_SOCKADDR("flom_tcp_listen: ai_addr ",
                                    (void *)sa, gai->ai_addrlen);
                if (-1 == (fd = socket(gai->ai_family, gai->ai_socktype,
                                       gai->ai_protocol))) {
                    FLOM_TRACE(("flom_tcp_listen/socket(): "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                } else if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                            (void *)&sock_opt,
                                            sizeof(sock_opt))) {
                    FLOM_TRACE(("flom_tcp_listen/setsockopt() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                } else if (-1 == bind(fd, sa, gai->ai_addrlen)) {
                    FLOM_TRACE(("flom_tcp_listen/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                } else {
                    bound = TRUE;
                    FLOM_TRACE(("flom_tcp_listen: bound!\n"));
                }
            } /* while (NULL != gai && !bound) */
            if (!bound)
                THROW(BIND_ERROR);
        }        
        if (-1 == listen(fd, LISTEN_BACKLOG))
            THROW(LISTEN_ERROR);
        /* set output values */
        obj->sockfd = fd;
        obj->addrlen = gai->ai_addrlen;
        memcpy(&obj->sa_storage, sa, obj->addrlen);
        fd = FLOM_NULL_FD; /* avoid socket close by clean-up section */        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
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
    if (NULL != result)
        freeaddrinfo(result);
    if (FLOM_NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_tcp_listen/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



const struct addrinfo *flom_tcp_try_connect(
    flom_config_t *config, const struct addrinfo *gai, int *fd)
{
    const struct addrinfo *found = NULL; 
    *fd = FLOM_NULL_FD;
    /* traverse the list and try to connect... */
    while (NULL != gai && NULL == found) {
        struct sockaddr_in6 sa6;
        struct sockaddr *sa = gai->ai_addr;
        /* IPv6 addresses could need sin6_scope_id set if the user specified
           a network interface */
        FLOM_TRACE_SOCKADDR("flom_tcp_try_connect: sa ",
                            sa, gai->ai_addrlen);
        if (AF_INET6 == sa->sa_family &&
            NULL != flom_config_get_network_interface(config)) {
            memcpy(&sa6, sa, gai->ai_addrlen);
            sa6.sin6_scope_id = flom_config_get_sin6_scope_id(config);
            sa = (struct sockaddr *)&sa6;
            FLOM_TRACE(("flom_tcp_try_connect: overriding field "
                        "sin6_scope_id with value %u\n", sa6.sin6_scope_id));
        }
            
        if (FLOM_NULL_FD == (*fd = socket(gai->ai_family, gai->ai_socktype,
                                          gai->ai_protocol))) {
            FLOM_TRACE(("flom_tcp_try_connect/socket(): "
                        "errno=%d '%s', skipping...\n", errno,
                        strerror(errno)));
            gai = gai->ai_next;
        } else {
            FLOM_TRACE_SOCKADDR("flom_tcp_try_connect: sa ",
                                sa, gai->ai_addrlen);
            if (-1 == connect(*fd, sa, gai->ai_addrlen)) {
                FLOM_TRACE(("flom_tcp_try_connect/connect(): "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(*fd);
                *fd = FLOM_NULL_FD;
            } else
                found = gai;
        } /* if (-1 == (*fd = socket( */
    } /* while (NULL != gai && !connected) */
    return found;
}



int flom_tcp_connect(flom_tcp_t *obj)
{
    enum Exception { GETADDRINFO_ERROR
                     , CONNECTION_REFUSED
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *result = NULL;
    int fd = FLOM_NULL_FD;
    
    FLOM_TRACE(("flom_tcp_connect\n"));
    TRY {
        struct addrinfo hints;
        const struct addrinfo *p = NULL;
        char port_string[100];
        int errcode;
        
        FLOM_TRACE(("flom_tcp_connect: connecting to address '%s' "
                    "and port %d\n",
                    flom_config_get_unicast_address(obj->config),
                    flom_config_get_unicast_port(obj->config)));
        memset(&hints, 0, sizeof(hints));

        hints.ai_flags = AI_CANONNAME;
        /* interface name is specified, IPv6 is forced */
        if (NULL != flom_config_get_network_interface(obj->config))
            hints.ai_family = AF_INET6;
        else
            hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(port_string, sizeof(port_string), "%u",
                 flom_config_get_unicast_port(obj->config));
        
        if (0 != (errcode = getaddrinfo(
                      flom_config_get_unicast_address(obj->config),
                      port_string, &hints, &result))) {
            FLOM_TRACE(("flom_tcp_connect/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } 
        FLOM_TRACE_ADDRINFO("flom_tcp_connect/getaddrinfo(): ",
                            result);
        if (NULL == (p = flom_tcp_try_connect(obj->config, result, &fd))) {
            /* domain must be set even if the connection failed because it's
               necessary to start a new daemon */
            obj->domain = result->ai_family;
            THROW(CONNECTION_REFUSED);
        }

        obj->domain = result->ai_family;
        obj->sockfd = fd;
        obj->socket_type = hints.ai_socktype;
        obj->addrlen = p->ai_addrlen;
        memcpy(&obj->sa_storage, p->ai_addr, obj->addrlen);
        FLOM_TRACE(("flom_tcp_connect: domain=%d, sockfd=%d, socket_type=%d, "
                    "addrlen=%u\n", obj->domain, obj->sockfd, obj->addrlen));
        FLOM_TRACE_SOCKADDR("flom_tcp_connect: ", &obj->sa, obj->addrlen);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case CONNECTION_REFUSED:
                ret_cod = FLOM_RC_CONNECTION_REFUSED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != result)
        freeaddrinfo(result);
    /* in case of error, close the socket */
    if (FLOM_RC_OK != ret_cod && FLOM_NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_tcp_connect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tcp_recv(const flom_tcp_t *obj, void *buf, size_t len,
                  size_t *received,
                  struct sockaddr *src_addr, socklen_t *addrlen)
{
    enum Exception {
        INVALID_SOCKET_TYPE,
        RECV_MSG_ERROR,
        RECVFROM_ERROR,
        NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_recv\n"));
    TRY {
        switch (obj->socket_type) {
            case SOCK_STREAM:
                if (FLOM_RC_OK != (ret_cod = flom_tcp_recv_msg(
                                       obj, buf, len, received)))
                    THROW(RECV_MSG_ERROR);
                break;
            case SOCK_DGRAM:
                if (0 > (*received = recvfrom(
                             obj->sockfd, buf, len, 0,
                             (struct sockaddr *)src_addr, addrlen)))
                    THROW(RECVFROM_ERROR);
                FLOM_TRACE_HEX_DATA("flom_tcp_recv: from ",
                                    (void *)src_addr, *addrlen);        
                break;
            default:
                THROW(INVALID_SOCKET_TYPE);
        } /* switch (type) */
        
        FLOM_TRACE(("flom_tcp_recv: fd=%d returned "
                    SSIZE_T_FORMAT " bytes '%*.*s'\n", obj->sockfd,
                    *received, *received, *received, buf));

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_SOCKET_TYPE:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case RECV_MSG_ERROR:
                break;
            case RECVFROM_ERROR:
                ret_cod = FLOM_RC_RECVFROM_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tcp_recv/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tcp_recv_msg(const flom_tcp_t *obj, char *buf, size_t len,
                      size_t *received)
{
    enum Exception {
        OUT_OF_RANGE,
        RECV_ERROR,
        BUFFER_OVERFLOW,
        NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_recv_msg\n"));
    TRY {
        char closing_tag[FLOM_MSG_BUFFER_SIZE];
        size_t retrieved = 0;
        size_t closing_tag_len;
        size_t to_be_read;
        ssize_t read_bytes;
        char closing_tag_last;
        int found = FALSE;
        int i, j;
        
        /* preparing the closing tag string */
        snprintf(closing_tag, sizeof(closing_tag), "</%s>",
                 FLOM_MSG_TAG_MSG);
        closing_tag_len = strlen(closing_tag);
        closing_tag_last = closing_tag[closing_tag_len-1];
        FLOM_TRACE(("flom_tcp_recv_msg: closing_tag='%s', closing_tag_len="
                    SIZE_T_FORMAT ", closing_tag_last='%c'\n",
                    closing_tag, closing_tag_len, closing_tag_last));
        if (len < closing_tag_len)
            THROW(OUT_OF_RANGE);
        /* loop until a complete message has been retrieved or an error
           occurs */
        to_be_read = closing_tag_len;
        while (!found) {
            read_bytes = recv(obj->sockfd, buf+retrieved, to_be_read, 0);
            FLOM_TRACE(("flom_tcp_recv_msg: read_bytes=%d '%*.*s'\n",
                        read_bytes, read_bytes, read_bytes, buf+retrieved));
            if (0 >= read_bytes)
                THROW(RECV_ERROR);
            retrieved += read_bytes;
            /* too few chars, go on */
            if (retrieved < closing_tag_len)
                continue;
            /* looping on closing_tag */
            j = 0;
            for (i=0; i<closing_tag_len; ++i) {
                if (buf[retrieved-j-1] == closing_tag[closing_tag_len-i-1])
                    j++;
                else
                    j = 0;
            } /* for (i=0; i<closing_tag_len; ++i) */
            if (j == closing_tag_len)
                found = TRUE;
            else {
                to_be_read = closing_tag_len - j;
                if (retrieved + to_be_read >= len)
                    THROW(BUFFER_OVERFLOW);
            }
        } /* while (TRUE) */
        /* put string terminator */
        buf[retrieved] = '\0';
        *received = retrieved;
        FLOM_TRACE(("flom_tcp_recv_msg: received message is '%s' "
                    "of " SIZE_T_FORMAT " chars\n", buf, *received));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case RECV_ERROR:
                ret_cod = FLOM_RC_RECV_ERROR;
                break;
            case BUFFER_OVERFLOW:
                ret_cod = FLOM_RC_BUFFER_OVERFLOW;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tcp_recv_msg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tcp_send(const flom_tcp_t *obj, const void *buf, size_t len)
{
    enum Exception { GETSOCKOPT_ERROR
                     , CONNECTION_CLOSED
                     , SEND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_send: fd=%d\n", obj->sockfd));
    TRY {
        ssize_t wrote_bytes;
        int optval;
        socklen_t optlen = sizeof(optval);

        if (0 != getsockopt(obj->sockfd, SOL_SOCKET, SO_ERROR,
                            &optval, &optlen))
            THROW(GETSOCKOPT_ERROR);
        FLOM_TRACE(("flom_tcp_send: so_error=%d (EPIPE=%d, ECONNRESET=%d)\n",
                    optval, EPIPE, ECONNRESET));
        if (EPIPE == optval || ECONNRESET == optval) {
            int rc = 0;
            rc = shutdown(obj->sockfd, SHUT_RDWR);
            FLOM_TRACE(("flom_tcp_send: socket with fd=%d was shutdown "
                        "(rc=%d,errno=%d)\n", obj->sockfd, rc, errno));
            THROW(CONNECTION_CLOSED);
        }
        FLOM_TRACE(("flom_tcp_send: sending " SIZE_T_FORMAT
                    " bytes (fd=%d) '%*.*s'...\n", len, obj->sockfd,
                    len, len, buf));
        wrote_bytes = send(obj->sockfd, buf, len, MSG_NOSIGNAL);
        if (len != wrote_bytes) {
            FLOM_TRACE(("flom_tcp_send: sent " SSIZE_T_FORMAT
                        " bytes instead of " SIZE_T_FORMAT "\n",
                        wrote_bytes, len));
            THROW(SEND_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_GETSOCKOPT_ERROR;
                break;
            case CONNECTION_CLOSED:
                ret_cod = FLOM_RC_CONNECTION_CLOSED;
                break;
            case SEND_ERROR:
                ret_cod = FLOM_RC_SEND_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tcp_send/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tcp_close(flom_tcp_t *obj)
{
    enum Exception { CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_close\n"));
    TRY {
        if (FLOM_NULL_FD == obj->sockfd) {
            FLOM_TRACE(("flom_tcp_close: sockfd is NULL, skipping...\n"));
        } else {
            if (0 != close(obj->sockfd))
                THROW(CLOSE_ERROR);
            obj->sockfd = FLOM_NULL_FD;
        } /* if (FLOM_NULL_FD == obj->sockfd) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
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
    FLOM_TRACE(("flom_tcp_close/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



gchar *flom_tcp_retrieve_peer_name(const flom_tcp_t *obj)
{
    struct sockaddr_storage sa;
    socklen_t addrlen = sizeof(sa);
    char host[NI_MAXHOST+1];
    char serv[NI_MAXSERV+1];
    int ret_cod;
    char *tmp;
    size_t tmp_size;

    memset(&sa, 0, sizeof(sa));
    if (0 != getpeername(obj->sockfd, (struct sockaddr *)&sa, &addrlen)) {
        FLOM_TRACE(("flom_tcp_retrieve_peer_name/getpeername: errno=%d\n",
                    errno));
        return NULL;
    }
    if (0 != (ret_cod = getnameinfo(
                  (struct sockaddr *)&sa, addrlen,
                  host, sizeof(host), serv, sizeof(serv),
                  NI_NUMERICHOST|NI_NUMERICSERV))) {
        FLOM_TRACE(("flom_tcp_retrieve_peer_name/getnameinfo: ret_cod=%d, "
                    "errno=%d\n", ret_cod, errno));
        return NULL;
    }
    tmp_size = strlen(host) + strlen(serv) + 2;
    if (NULL != (tmp = g_try_malloc0(tmp_size))) {
        snprintf(tmp, tmp_size, "%s/%s", host, serv);
    }
    return tmp;
}
