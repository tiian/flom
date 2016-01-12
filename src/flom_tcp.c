/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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



int flom_tcp_retrieve(int fd, int type, char *buf, size_t buf_size,
                      ssize_t *read_bytes, int timeout,
                      struct sockaddr *src_addr, socklen_t *addrlen)
{
    enum Exception { POLL_ERROR
                     , NETWORK_TIMEOUT
                     , INTERNAL_ERROR
                     , INVALID_SOCKET_TYPE
                     , RECV_ERROR
                     , RECVFROM_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_retrieve\n"));
    TRY {
        if (timeout >= 0) {
            struct pollfd fds[1];
            int rc;
            /* use poll to check the filedescriptor for a limited amount of
               time */
            fds[0].fd = fd;
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
        } /* if (timeout >= 0) */

        switch (type) {
            case SOCK_STREAM:
                if (0 > (*read_bytes = recv(fd, buf, buf_size, 0)))
                    THROW(RECV_ERROR);
                break;
            case SOCK_DGRAM:
                if (0 > (*read_bytes = recvfrom(
                             fd, buf, buf_size, 0,
                             (struct sockaddr *)src_addr, addrlen)))
                    THROW(RECVFROM_ERROR);
                FLOM_TRACE_HEX_DATA("flom_tcp_retrieve: from ",
                                    (void *)src_addr, *addrlen);        
                break;
            default:
                THROW(INVALID_SOCKET_TYPE);
        } /* switch (type) */
        
        FLOM_TRACE(("flom_tcp_retrieve: fd=%d returned "
                    SSIZE_T_FORMAT " bytes '%*.*s'\n", fd, *read_bytes,
                    *read_bytes, *read_bytes, buf));

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
            case INVALID_SOCKET_TYPE:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case RECV_ERROR:
                ret_cod = FLOM_RC_RECV_ERROR;
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
    FLOM_TRACE(("flom_tcp_retrieve/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tcp_send(int fd, const char *buf, size_t buf_size)
{
    enum Exception { GETSOCKOPT_ERROR
                     , CONNECTION_CLOSED
                     , SEND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tcp_send: fd=%d\n", fd));
    TRY {
        ssize_t wrote_bytes;
        int optval;
        socklen_t optlen = sizeof(optval);

        if (0 != getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen))
            THROW(GETSOCKOPT_ERROR);
        FLOM_TRACE(("flom_tcp_send: so_error=%d (EPIPE=%d, ECONNRESET=%d)\n",
                    optval, EPIPE, ECONNRESET));
        if (EPIPE == optval || ECONNRESET == optval) {
            int rc = 0;
            rc = shutdown(fd, SHUT_RDWR);
            FLOM_TRACE(("flom_tcp_send: socket with fd=%d was shutdown "
                        "(rc=%d,errno=%d)\n", fd, rc, errno));
            THROW(CONNECTION_CLOSED);
        }
        FLOM_TRACE(("flom_tcp_send: sending " SIZE_T_FORMAT
                    " bytes (fd=%d) '%*.*s'...\n", buf_size, fd,
                    buf_size, buf_size, buf));
        wrote_bytes = send(fd, buf, buf_size, MSG_NOSIGNAL);
        if (buf_size != wrote_bytes) {
            FLOM_TRACE(("flom_tcp_send: sent " SSIZE_T_FORMAT
                        " bytes instead of " SIZE_T_FORMAT "\n",
                        wrote_bytes, buf_size));
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



