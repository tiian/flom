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



int flom_tcp_listen(flom_config_t *config, int domain,
                    int *sockfd, size_t *addrlen, struct sockaddr *address)
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
        hints.ai_family = domain;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_unicast_port(config));
        FLOM_TRACE(("flom_tcp_listen: binding address '%s' "
                    "and port %s\n", flom_config_get_unicast_address(config),
                    port));

        if (0 != (errcode = getaddrinfo(
                      flom_config_get_unicast_address(config),
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
                    NULL != flom_config_get_network_interface(config)) {
                    memcpy(&sa6, sa, gai->ai_addrlen);
                    sa6.sin6_scope_id = flom_config_get_sin6_scope_id(config);
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
        *sockfd = fd;
        *addrlen = gai->ai_addrlen;
        memcpy(address, sa, *addrlen);
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

