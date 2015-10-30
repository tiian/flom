/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_debug_features.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DEBUG_FEATURES



const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_SERVER = _DEBUG_FEATURES_IPV6_MULTICAST_SERVER;
const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT = _DEBUG_FEATURES_IPV6_MULTICAST_CLIENT;



int flom_debug_features(const char *name)
{
    enum Exception { INVALID_OPTION
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_debug_features\n"));
    TRY {
        FLOM_TRACE(("flom_debug_features: name='%s'\n", name));

        if (0 == strcasecmp(name, FLOM_DEBUG_FEATURES_IPV6_MULTICAST_SERVER))
            ret_cod = flom_debug_features_ipv6_multicast_server();
        else if (0 == strcasecmp(name,
                                 FLOM_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT))
            ret_cod = flom_debug_features_ipv6_multicast_client();
        else {
            FLOM_TRACE(("flom_debug_features: debug feature '%s' "
                        "is not available\n", name));
            THROW(INVALID_OPTION);
        }
                    
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                break;
            case INVALID_OPTION:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_debug_features/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_debug_features_ipv6_multicast_server(void)
{
    enum Exception { GETADDRINFO_ERROR
                     , INVALID_IP_ADDRESS
                     , RECVFROM_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *res = NULL;
    int fd = FLOM_NULL_FD;

    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server\n"));
    TRY {
        char port[10];
        struct addrinfo hints;
        int errcode;
        const struct addrinfo *gai;
        const int sock_opt = 1;
        struct sockaddr_in6 bind_addr;
        socklen_t bind_addr_len;
        struct ipv6_mreq mreq6;
        ssize_t read_bytes;
        char buf[2048];
        struct sockaddr src_addr;
        socklen_t src_addr_len;

        /* prepare port for getaddrinfo(...) */
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_multicast_port(NULL));
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "multicast address='%s', multicast port=%s\n",
                    STRORNULL(flom_config_get_multicast_address(NULL)),
                    port));

        /* prepare hints for getaddrinfo(...) */
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        if (0 != (errcode = getaddrinfo(
                      flom_config_get_multicast_address(NULL), port,
                      &hints, &res))) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                        "getaddrinfo(): errcode=%d '%s'\n",
                        errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } /* if (0 != (errcode = getaddrinfo( */

        /* traverse the list returned by getaddrinfo */
        gai = res;
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                    "getaddrinfo() list pointer %p\n", gai));
        while (NULL != gai) {
            /* debugging address */
            FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_server: "
                                "address returned by getaddrinfo():",
                                gai->ai_addr, gai->ai_addrlen);
            FLOM_TRACE_ADDRINFO("flom_debug_features_ipv6_multicast_server/"
                                "getaddrinfo(): ", gai);
            /* creating socket */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server:"
                        " creating a new socket...\n"));
            if (-1 == (fd = socket(gai->ai_family, gai->ai_socktype,
                                   gai->ai_protocol))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "socket(): errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                continue; /* trying next address if available */
            } else {
                /* debugging the retrieved socket */
                struct sockaddr socket_addr;
                socklen_t socket_addrlen;
                if (-1 == getsockname(fd, &socket_addr, &socket_addrlen)) {
                    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                                "getsockname(): errno=%d '%s', skipping...\n",
                                errno, strerror(errno)));
                    gai = gai->ai_next;
                    continue; /* trying next address if available */
                } else {
                    FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_"
                                        "server: address returned by "
                                        "getsockname():",
                                        &socket_addr, socket_addrlen);
                } /* if (-1 == getsockname(fd */
            }
            /* setting socket property */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server:"
                        " setting SO_REUSEADDR socket property...\n"));
            if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                 (void *)&sock_opt, sizeof(sock_opt))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "setsockopt(SO_REUSEADDR): "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fd);
                fd = FLOM_NULL_FD;
                continue; /* trying next address if available */
            }
            /* binding local port */
            bind_addr_len = sizeof(bind_addr);
            memset(&bind_addr, 0, bind_addr_len);
            bind_addr.sin6_family = AF_INET6;
            bind_addr.sin6_addr = in6addr_any;
            bind_addr.sin6_port = htons(flom_config_get_multicast_port(NULL));
            FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_server:"
                                " binding to address ",
                                (struct sockaddr *)&bind_addr,
                                bind_addr_len);
            if (-1 == bind(fd, (struct sockaddr *)&bind_addr, bind_addr_len)) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "bind: errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fd);
                fd = FLOM_NULL_FD;
                continue; /* trying next address if available */
            } else {
                /* debugging the bound socket */
                struct sockaddr socket_addr;
                socklen_t socket_addrlen;
                if (-1 == getsockname(fd, &socket_addr, &socket_addrlen)) {
                    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                                "getsockname(): errno=%d '%s', skipping...\n",
                                errno, strerror(errno)));
                    gai = gai->ai_next;
                    continue; /* trying next address if available */
                } else {
                    FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_"
                                        "server: address returned by "
                                        "bind():",
                                        &socket_addr, socket_addrlen);
                } /* if (-1 == getsockname(fd */
            }
            /* activating multicast */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server:"
                        " activating multicast...\n"));
            memcpy(&mreq6.ipv6mr_multiaddr,
                   &((struct sockaddr_in6 *)gai->ai_addr)->sin6_addr,
                   sizeof(struct in6_addr));
            /* all the interfaces... */
            mreq6.ipv6mr_interface = 0;
            if (-1 == setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                                 &mreq6, sizeof(mreq6))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "setsockopt(IPV6_ADD_MEMBERSHIP): "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fd);
                fd = FLOM_NULL_FD;
                continue; /* trying next address if available */
            }
            /* that's OK, break the loop */
            break;
            gai = gai->ai_next;
        } /* while (NULL != gai) */
        if (NULL == gai) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                        "unable to create a multicast server\n"));
            THROW(INVALID_IP_ADDRESS);
        }
        
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "multicast server created, waiting datagram...\n"));
        /* reset source address struct */
        src_addr_len = sizeof(src_addr);
        memset(&src_addr, 0, src_addr_len);
        if (0 > (read_bytes = recvfrom(fd, buf, sizeof(buf), 0,
                                       &src_addr, &src_addr_len))) {
            THROW(RECVFROM_ERROR);
        }
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "arrived datagram '%*.*s'\n",
                    read_bytes, read_bytes, buf));
        FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_server: "
                            "address returned by recvfrom(): ",
                            &src_addr, src_addr_len);
        
        /* @@@ going on with code from flom_accept_discover_reply... */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case INVALID_IP_ADDRESS:
                ret_cod = FLOM_RC_INVALID_IP_ADDRESS;
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
    /* release getaddrinfo data */
    if (NULL != res)
        freeaddrinfo(res);
    if (FLOM_NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_debug_features_ipv6_multicast_client(void)
{
    enum Exception { GETADDRINFO_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct addrinfo *res = NULL;
    
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_client\n"));
    TRY {
        char port[10];
        struct addrinfo hints;
        int errcode;
        const struct addrinfo *gai;
        
        /* prepare port for getaddrinfo(...) */
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_multicast_port(NULL));
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                    "multicast address='%s', multicast port=%s\n",
                    STRORNULL(flom_config_get_multicast_address(NULL)),
                    port));

        /* prepare hints for getaddrinfo(...) */
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        if (0 != (errcode = getaddrinfo(
                      flom_config_get_multicast_address(NULL), port,
                      &hints, &res))) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                        "getaddrinfo(): errcode=%d '%s'\n",
                        errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } /* if (0 != (errcode = getaddrinfo( */
        /* traverse the list returned by getaddrinfo */
        gai = res;
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                    "getaddrinfo() list pointer %p\n", gai));
        while (NULL != gai) {
            struct sockaddr_in6 local_addr;
            socklen_t local_addr_len;
            
            /* debugging address */
            FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_client: "
                                "address returned by getaddrinfo():",
                                gai->ai_addr, gai->ai_addrlen);
            FLOM_TRACE_ADDRINFO("flom_debug_features_ipv6_multicast_client/"
                                "getaddrinfo(): ", gai);

            /* preparing a local address on an ephemeral port to listen for
               a reply from server */
            local_addr_len = sizeof(local_addr);
            memset(&local_addr, 0, local_addr_len);
            local_addr.sin6_family = gai->ai_family;
            local_addr.sin6_addr = in6addr_any;
            local_addr.sin6_port = 0;
            FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_client: "
                                "local address to listen for a reply from "
                                "server:", (struct sockaddr *)&local_addr,
                                local_addr_len);
            /* @@@
               go on with code from flom_client_discover_udp...
               check the socket creation sequence before "sendto" function
               and check with Richard Stevens' book...
            */
            gai = gai->ai_next;
        } /* while (NULL != gai) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release getaddrinfo data */
    if (NULL != res)
        freeaddrinfo(res);
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

