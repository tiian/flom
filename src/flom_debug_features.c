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
#include "flom_tls.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DEBUG_FEATURES



const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_SERVER = _DEBUG_FEATURES_IPV6_MULTICAST_SERVER;
const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT = _DEBUG_FEATURES_IPV6_MULTICAST_CLIENT;
const char *FLOM_DEBUG_FEATURES_TLS_SERVER = _DEBUG_FEATURES_TLS_SERVER;
const char *FLOM_DEBUG_FEATURES_TLS_CLIENT = _DEBUG_FEATURES_TLS_CLIENT;



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
        else if (0 == strcasecmp(name, FLOM_DEBUG_FEATURES_TLS_SERVER))
            ret_cod = flom_debug_features_tls_server();
        else if (0 == strcasecmp(name,
                                 FLOM_DEBUG_FEATURES_TLS_CLIENT))
            ret_cod = flom_debug_features_tls_client();
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
                     , SOCKET_ERROR
                     , SENDTO_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *res = NULL;
    int fdi = FLOM_NULL_FD; /* file descriptor for incoming datagrams */

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
        struct sockaddr_storage src_addr;
        socklen_t src_addr_len;
        const char welcome_msg[] = "WELCOME";
        size_t welcome_msg_len = strlen(welcome_msg);
        ssize_t sent_bytes;
        
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
            if (-1 == (fdi = socket(gai->ai_family, gai->ai_socktype,
                                   gai->ai_protocol))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "socket(): errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                continue; /* trying next address if available */
            } else {
                /* debugging the retrieved socket */
                struct sockaddr_storage socket_addr;
                socklen_t socket_addrlen = sizeof(socket_addr);
                if (-1 == getsockname(fdi, (struct sockaddr *)&socket_addr,
                                      &socket_addrlen)) {
                    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                                "getsockname(): errno=%d '%s', skipping...\n",
                                errno, strerror(errno)));
                    gai = gai->ai_next;
                    continue; /* trying next address if available */
                } else {
                    FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_"
                                        "server: address returned by "
                                        "getsockname(): ",
                                        (struct sockaddr *)&socket_addr,
                                        socket_addrlen);
                } /* if (-1 == getsockname(fdi */
            }
            /* setting socket property */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server:"
                        " setting SO_REUSEADDR socket property...\n"));
            if (-1 == setsockopt(fdi, SOL_SOCKET, SO_REUSEADDR,
                                 (void *)&sock_opt, sizeof(sock_opt))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "setsockopt(SO_REUSEADDR): "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fdi);
                fdi = FLOM_NULL_FD;
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
            if (-1 == bind(fdi, (struct sockaddr *)&bind_addr,
                           bind_addr_len)) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "bind: errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fdi);
                fdi = FLOM_NULL_FD;
                continue; /* trying next address if available */
            } else {
                /* debugging the bound socket */
                struct sockaddr_storage socket_addr;
                socklen_t socket_addrlen = sizeof(socket_addr);
                if (-1 == getsockname(fdi, (struct sockaddr *)&socket_addr,
                                      &socket_addrlen)) {
                    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                                "getsockname(): errno=%d '%s', skipping...\n",
                                errno, strerror(errno)));
                    gai = gai->ai_next;
                    continue; /* trying next address if available */
                } else {
                    FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_"
                                        "server: address returned by "
                                        "bind() ",
                                        (struct sockaddr *)&socket_addr,
                                        socket_addrlen);
                } /* if (-1 == getsockname(fdi */
            }
            /* activating multicast */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server:"
                        " activating multicast...\n"));
            memcpy(&mreq6.ipv6mr_multiaddr,
                   &((struct sockaddr_in6 *)gai->ai_addr)->sin6_addr,
                   sizeof(struct in6_addr));
            /* all the interfaces... */
            mreq6.ipv6mr_interface = 0;
            if (-1 == setsockopt(fdi, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                                 &mreq6, sizeof(mreq6))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "setsockopt(IPV6_ADD_MEMBERSHIP): "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fdi);
                fdi = FLOM_NULL_FD;
                continue; /* trying next address if available */
            }
            /* that's OK, break the loop */
            break;
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
        if (0 > (read_bytes = recvfrom(fdi, buf, sizeof(buf), 0,
                                       (struct sockaddr *)&src_addr,
                                       &src_addr_len))) {
            THROW(RECVFROM_ERROR);
        }
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "arrived datagram '%*.*s'\n",
                    read_bytes, read_bytes, buf));
        FLOM_TRACE_HEX_DATA("flom_debug_features_ipv6_multicast_server: "
                            "arrived datagram ", (byte_t *)buf, read_bytes);
        FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_server: "
                            "address returned by recvfrom() ",
                            (struct sockaddr *)&src_addr, src_addr_len);
        printf("Arrived datagram is '%*.*s'\n",
               (int)read_bytes, (int)read_bytes, buf);

        /* send reply message */
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "sending '%s' to the client...\n", welcome_msg));
        sent_bytes = sendto(fdi, welcome_msg, welcome_msg_len, 0,
                            (struct sockaddr *)&src_addr,
                            src_addr_len);
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_server: "
                    "sent_bytes=%d, welcome_msg_len=%d\n",
                    sent_bytes, welcome_msg_len));
        if (welcome_msg_len != sent_bytes) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                        "sendto(): sent %d instead of %d bytes; "
                        "errno=%d '%s'\n",
                        sent_bytes, welcome_msg_len,
                        errno, strerror(errno)));
            THROW(SENDTO_ERROR);
        }
        printf("Sent datagram is '%s'\n", welcome_msg);
        
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
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case SENDTO_ERROR:
                ret_cod = FLOM_RC_SENDTO_ERROR;
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
    if (FLOM_NULL_FD != fdi)
        close(fdi);
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_debug_features_ipv6_multicast_client(void)
{
    enum Exception { GETADDRINFO_ERROR
                     , INVALID_IP_ADDRESS
                     , SETSOCKOPT_ERROR
                     , SOCKET_ERROR
                     , BIND_ERROR
                     , GETSOCKNAME_ERROR
                     , SENDTO_ERROR
                     , RECVFROM_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct addrinfo *res = NULL;
    int fdo = FLOM_NULL_FD; /* file descriptor for outcoming datagrams */
    
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_client\n"));
    TRY {
        char port[10];
        struct addrinfo hints;
        int errcode;
        const struct addrinfo *gai;
        const char hello_msg[] = "HELLO";
        size_t hello_msg_len = strlen(hello_msg);
        ssize_t sent_bytes;
        ssize_t read_bytes;
        char buf[2048];
        struct sockaddr_storage src_addr;
        socklen_t src_addr_len;
        int sock_opt1 = 1;
        struct sockaddr_in6 local_addr;
        socklen_t local_addr_len;
        
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
            int ipv6_hops = flom_config_get_discovery_ttl(NULL);
            
            /* debugging address */
            FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_client: "
                                "address returned by getaddrinfo():",
                                gai->ai_addr, gai->ai_addrlen);
            FLOM_TRACE_ADDRINFO("flom_debug_features_ipv6_multicast_client/"
                                "getaddrinfo(): ", gai);

            /* creating a socket for sending a datagram to the server */
            if (FLOM_NULL_FD == (fdo = socket(gai->ai_family, gai->ai_socktype,
                                              gai->ai_protocol))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                            "socket(): errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                continue;
            }
            
            /* set IPV6_MULTICAST_HOPS socket option */
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                        "setting IPV6_MULTICAST_HOPS to value %d\n",
                        ipv6_hops));
            if (-1 == setsockopt(fdo, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                                 (void *)&ipv6_hops, sizeof(ipv6_hops))) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                            "setsockopt(IPV6_MULTICAST_HOPS) : "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(fdo);
                fdo = FLOM_NULL_FD;
                continue;
            }
            
            /* that's OK, break the loop */
            break;
        } /* while (NULL != gai) */
        if (NULL == gai) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                        "unable to create a multicast client\n"));
            THROW(INVALID_IP_ADDRESS);
        }

        /* set SO_REUSEADDR socket option, just in case a process crashed
           just before */
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                    "setting SO_REUSEADDR to value %d\n", sock_opt1));
        if (-1 == setsockopt(fdo, SOL_SOCKET, SO_REUSEADDR,
                             (void *)&sock_opt1, sizeof(sock_opt1))) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                        "setsockopt(SO_REUSEADDR) : "
                        "errno=%d '%s', skipping...\n", errno,
                        strerror(errno)));
            THROW(SETSOCKOPT_ERROR);
        }

        /* preparing a local address on an ephemeral port to listen for
           a reply from server */
        local_addr_len = sizeof(local_addr);
        memset(&local_addr, 0, local_addr_len);
        local_addr.sin6_family = gai->ai_family;
        local_addr.sin6_addr = in6addr_any;
        local_addr.sin6_port = 0;
        FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_client: "
                            "binding to address: ",
                            (struct sockaddr *)&local_addr,
                            local_addr_len);
        
        /* binding socket to local address */
        if (-1 == bind(fdo, (struct sockaddr *)&local_addr,
                       local_addr_len)) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                        "bind() : errno=%d '%s', skipping...\n",
                        errno, strerror(errno)));
            THROW(BIND_ERROR);
        } else {
            /* debugging the bound socket */
            struct sockaddr_storage socket_addr;
            socklen_t socket_addrlen;
            memset(&socket_addr, 0, sizeof(socket_addr));
            if (-1 == getsockname(fdo, (struct sockaddr *)&socket_addr,
                                  &socket_addrlen)) {
                FLOM_TRACE(("flom_debug_features_ipv6_multicast_server/"
                            "getsockname(): errno=%d '%s'\n",
                            errno, strerror(errno)));
                THROW(GETSOCKNAME_ERROR);
            } else {
                FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_"
                                    "client: address returned by "
                                    "bind() ",
                                    (struct sockaddr *)&socket_addr,
                                    socket_addrlen);
            } /* if (-1 == getsockname(fdi */
        } /* if (-1 == bind */
            
        /* send hello message to the server */
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                    "sending '%s' to the server...\n", hello_msg));
        FLOM_TRACE_SOCKADDR("flom_debug_features_ipv6_multicast_client: "
                            "destination address:",
                            gai->ai_addr, gai->ai_addrlen);
        if (hello_msg_len != (sent_bytes = sendto(
                                  fdo, hello_msg, hello_msg_len, 0,
                                  gai->ai_addr,
                                  gai->ai_addrlen))) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                        "sendto(): sent %d instead of %d bytes; "
                        "errno=%d '%s'\n",
                        sent_bytes, hello_msg_len,
                        errno, strerror(errno)));
            THROW(SENDTO_ERROR);
        }
        printf("Sent datagram is '%s'\n", hello_msg);

        /* wait server response */
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                    "waiting an answer from the server...\n"));
        src_addr_len = sizeof(src_addr);
        memset(&src_addr, 0, src_addr_len);
        if (-1 == (read_bytes = recvfrom(fdo, buf, sizeof(buf), 0,
                                         (struct sockaddr *)&src_addr,
                                         &src_addr_len))) {
            FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/"
                        "recvfrom(): errno=%d '%s'\n",
                        errno, strerror(errno)));
            THROW(RECVFROM_ERROR);
        }
        FLOM_TRACE(("flom_debug_features_ipv6_multicast_client: "
                    "received '%*.*s'\n", read_bytes, read_bytes, buf));
        printf("Arrived datagram is '%*.*s'\n",
               (int)read_bytes, (int)read_bytes, buf);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case INVALID_IP_ADDRESS:
                ret_cod = FLOM_RC_INVALID_IP_ADDRESS;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case BIND_ERROR:
                ret_cod = FLOM_RC_BIND_ERROR;
                break;
            case GETSOCKNAME_ERROR:
                ret_cod = FLOM_RC_GETSOCKNAME_ERROR;
                break;
            case SENDTO_ERROR:
                ret_cod = FLOM_RC_SENDTO_ERROR;
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
    if (FLOM_NULL_FD != fdo)
        close(fdo);
    FLOM_TRACE(("flom_debug_features_ipv6_multicast_client/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_debug_features_tls_server(void)
{
    enum Exception { TLS_CREATE_CONTEXT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_debug_features_tls_server\n"));
    TRY {
        flom_tls_t tls;
        
        /* initialize TLS/SSL support */
        flom_tls_init(&tls, FALSE);
        
        /* create a TLS/SSL context */
        if (FLOM_RC_OK != (ret_cod = flom_tls_context(&tls)))
            THROW(TLS_CREATE_CONTEXT_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_CREATE_CONTEXT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_debug_features_tls_server/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_debug_features_tls_client(void)
{
    enum Exception { TLS_CREATE_CONTEXT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_debug_features_tls_client\n"));
    TRY {
        flom_tls_t tls;
        
        /* initialize TLS/SSL support */
        flom_tls_init(&tls, TRUE);

        /* create a TLS/SSL context */
        if (FLOM_RC_OK != (ret_cod = flom_tls_context(&tls)))
            THROW(TLS_CREATE_CONTEXT_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_CREATE_CONTEXT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_debug_features_tls_client/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

