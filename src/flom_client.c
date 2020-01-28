/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_client.h"
#include "flom_daemon.h"
#include "flom_errors.h"
#include "flom_msg.h"
#include "flom_syslog.h"
#include "flom_tcp.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CLIENT



int flom_client_connect(flom_config_t *config,
                        flom_conn_t *conn, int start_daemon)
{
    enum Exception { CLIENT_CONNECT_LOCAL_ERROR
                     , CLIENT_CONNECT_TCP_ERROR
                     , DAEMON_ERROR
                     , CLIENT_DISCOVER_UDP_ERROR1
                     , CONNECT_ERROR
                     , DAEMON_NOT_STARTED
                     , CLIENT_DISCOVER_UDP_ERROR2
                     , INTERNAL_ERROR
                     , FCNTL_ERROR
                     , TLS_CREATE_CONTEXT_ERROR
                     , TLS_SET_CERT_ERROR
                     , TLS_CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_connect\n"));
    TRY {
        /* choose and instantiate connection type */
        if (NULL != flom_config_get_socket_name(config)) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect_local(
                                   config, conn, start_daemon)))
                THROW(CLIENT_CONNECT_LOCAL_ERROR);
        } else if (NULL != flom_config_get_unicast_address(config)) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect_tcp(
                                   config, conn, start_daemon)))
                THROW(CLIENT_CONNECT_TCP_ERROR);
        } else if (NULL != flom_config_get_multicast_address(config)) {
            sa_family_t family;
            ret_cod = flom_client_discover_udp(
                config, conn, start_daemon, &family);
            switch (ret_cod) {
                case FLOM_RC_OK:
                    break;
                case FLOM_RC_CONNECTION_REFUSED:
                    if (start_daemon) {
                        if (0 != flom_config_get_lifespan(config)) {
                            FLOM_TRACE(("flom_client_connect: connection "
                                        "failed, activating a new daemon\n"));
                            /* try to start a daemon on this node */
                            if (FLOM_RC_OK != (ret_cod = flom_daemon(
                                                   config, family)))
                                THROW(DAEMON_ERROR);
                            /* try to discover again */
                            if (FLOM_RC_OK != (
                                    ret_cod = flom_client_discover_udp(
                                        config, conn, start_daemon,
                                        &family)))
                                THROW(CLIENT_DISCOVER_UDP_ERROR1);
                        } else {
                            FLOM_TRACE(("flom_client_connect: connection "
                                        "failed, a new daemon can not be "
                                        "started "
                                        "because daemon lifespan is 0\n"));
                            THROW(CONNECT_ERROR);
                        }
                    } else {
                        THROW(DAEMON_NOT_STARTED);
                    } /* if (start_daemon) */
                    break;
                default:
                    THROW(CLIENT_DISCOVER_UDP_ERROR2);
            } /* switch (ret_cod) */
        } else /* this condition must be impossible! */
            THROW(INTERNAL_ERROR);
        /* set CLOSE on EXEC to avoid this file descriptor remains open after
           execution of controlled child command */
        if (-1 == fcntl(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                                            F_SETFD, FD_CLOEXEC))
            THROW(FCNTL_ERROR);
        FLOM_TRACE(("flom_client_connect: set FD_CLOEXEC to fd=%d\n",
                    flom_tcp_get_sockfd(flom_conn_get_tcp(conn))));

        /* switch to TLS? */
        if (NULL != flom_config_get_tls_certificate(config) &&
            NULL != flom_config_get_tls_private_key(config) &&
            NULL != flom_config_get_tls_ca_certificate(config)) {
            /* initialize TLS/SSL support */
            flom_conn_init_tls(conn, TRUE);

            /* create a TLS/SSL context */
            if (FLOM_RC_OK != (ret_cod = flom_tls_context(
                                   flom_conn_get_tls(conn))))
                THROW(TLS_CREATE_CONTEXT_ERROR);
            
            /* set certificates */
            if (FLOM_RC_OK != (
                    ret_cod = flom_tls_set_cert(
                        flom_conn_get_tls(conn),
                        flom_config_get_tls_certificate(config),
                        flom_config_get_tls_private_key(config),
                        flom_config_get_tls_ca_certificate(config))))
                THROW(TLS_SET_CERT_ERROR);
            
            /* switch the client connection to TLS */
            if (FLOM_RC_OK != (ret_cod = flom_tls_connect(
                                   flom_conn_get_tls(conn),
                                   flom_tcp_get_sockfd(
                                       flom_conn_get_tcp(conn)))))
                THROW(TLS_CONNECT_ERROR);
        } /* switched to TLS! */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CLIENT_CONNECT_LOCAL_ERROR:
            case CLIENT_CONNECT_TCP_ERROR:
            case DAEMON_ERROR:
            case CLIENT_DISCOVER_UDP_ERROR1:
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case DAEMON_NOT_STARTED:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case CLIENT_DISCOVER_UDP_ERROR2:
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case FCNTL_ERROR:
                ret_cod = FLOM_RC_FCNTL_ERROR;
                break;
            case TLS_CREATE_CONTEXT_ERROR:
            case TLS_SET_CERT_ERROR:
            case TLS_CONNECT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_client_connect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_connect_local(flom_config_t *config,
                              flom_conn_t *conn,
                              int start_daemon)
{
    enum Exception { SOCKET_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED1
                     , CONNECT_ERROR1
                     , DAEMON_NOT_STARTED2
                     , CONNECT_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_connect_local\n"));
    TRY {
        FLOM_TRACE(("flom_client_connect_local: connecting to socket '%s'\n",
                    flom_config_get_socket_name(config)));

        if (-1 == (flom_tcp_set_sockfd(
                       flom_conn_get_tcp(conn),
                       socket(AF_LOCAL, SOCK_STREAM, 0))))
            THROW(SOCKET_ERROR);
        flom_tcp_set_socket_type(flom_conn_get_tcp(conn), SOCK_STREAM);
        flom_tcp_get_sa_un(flom_conn_get_tcp(conn))->sun_family = AF_LOCAL;
        strcpy(flom_tcp_get_sa_un(flom_conn_get_tcp(conn))->sun_path,
               flom_config_get_socket_name(config));
        flom_tcp_set_addrlen(flom_conn_get_tcp(conn),
                             sizeof(struct sockaddr_un));
        if (-1 == connect(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                          flom_tcp_get_sa(flom_conn_get_tcp(conn)),
                          flom_tcp_get_addrlen(flom_conn_get_tcp(conn)))) {
            if (ENOENT == errno || ECONNREFUSED == errno) {
                if (start_daemon) {
                    if (0 != flom_config_get_lifespan(config)) {
                        FLOM_TRACE(("flom_client_connect_local: connection "
                                    "failed, activating a new daemon\n"));
                        /* daemon is not active, starting it... */
                        ret_cod = flom_daemon(
                            config, flom_tcp_get_sa_un(
                                flom_conn_get_tcp(conn))->sun_family);
                        /* if previous call returns FLOM_RC_FLOCK_ERROR
                           there's probably another daemon that's starting */
                        if (FLOM_RC_OK != ret_cod &&
                            FLOM_RC_FLOCK_ERROR != ret_cod)
                            THROW(DAEMON_ERROR);
                        if (FLOM_RC_FLOCK_ERROR == ret_cod) {
                            FLOM_TRACE(("flom_client_connect_local: a new "
                                        "daemon can not be started because "
                                        "the synchronization file is already "
                                        "locked, maybe another daemon is "
                                        "starting...\n"));
                        }
                        /* trying to connect again... */
                        if (-1 == connect(
                                flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                                flom_tcp_get_sa(flom_conn_get_tcp(conn)),
                                flom_tcp_get_addrlen(flom_conn_get_tcp(conn))))
                            THROW(DAEMON_NOT_STARTED1);
                        FLOM_TRACE(("flom_client_connect_local: connected to "
                                    "flom daemon\n"));
                    } else {
                        FLOM_TRACE(("flom_client_connect_local: connection "
                                    "failed, a new daemon can not be started "
                                    "because daemon lifespan is 0\n"));
                        THROW(CONNECT_ERROR1);
                    } /* if (0 != flom_config_get_lifespan()) */
                } else {
                    THROW(DAEMON_NOT_STARTED2);
                } /* if (start_daemon) */
            } else {
                THROW(CONNECT_ERROR2);
            }
        } /* if (-1 == connect(cd->fd, */

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case DAEMON_ERROR:
                break;
            case DAEMON_NOT_STARTED1:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case CONNECT_ERROR1:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case DAEMON_NOT_STARTED2:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;                
            case CONNECT_ERROR2:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_client_connect_local/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_connect_tcp(flom_config_t *config,
                            flom_conn_t *conn, int start_daemon)
{
    enum Exception { TCP_CONNECT_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED1
                     , CONNECT_ERROR1
                     , DAEMON_NOT_STARTED2
                     , SETSOCKOPT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    
    FLOM_TRACE(("flom_client_connect_tcp\n"));
    TRY {
        flom_tcp_init(flom_conn_get_tcp(conn), config);
        int sock_opt = 1;

        ret_cod = flom_tcp_connect(flom_conn_get_tcp(conn));
        switch (ret_cod) {
            case FLOM_RC_OK:
                break;
            case FLOM_RC_CONNECTION_REFUSED:
                if (start_daemon) {
                    if (0 != flom_config_get_lifespan(config)) {
                        FLOM_TRACE(("flom_client_connect_tcp: connection "
                                    "failed, activating a new daemon\n"));
                        /* daemon is not active, starting it... */
                        ret_cod = flom_daemon(config,
                                              flom_tcp_get_domain(
                                                  flom_conn_get_tcp(conn)));
                        /* in the event of bind error, the daemon might have
                           been activated by someone else */
                        if (FLOM_RC_OK != ret_cod &&
                            FLOM_RC_BIND_ERROR != ret_cod)
                            THROW(DAEMON_ERROR);
                        /* trying to connect again... */
                        if (FLOM_RC_OK != (ret_cod = flom_client_connect_tcp(
                                               config, conn, FALSE))) {
                            THROW(DAEMON_NOT_STARTED1);
                        } else /* recursive call, bypass next steps */
                            THROW(NONE); 
                    } else {
                        FLOM_TRACE(("flom_client_connect_tcp: connection "
                                    "failed, a new daemon can not be started "
                                    "because daemon lifespan is 0\n"));
                        THROW(CONNECT_ERROR1);
                    }
                } else {
                    THROW(DAEMON_NOT_STARTED2)
                        } /* if (start_daemon) */
                break;
            default:
                THROW(TCP_CONNECT_ERROR);
        } /* switch (ret_cod) */
        if (0 != setsockopt(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                            IPPROTO_TCP, TCP_NODELAY,
                            (void *)(&sock_opt), sizeof(sock_opt)))
            THROW(SETSOCKOPT_ERROR);
        /* set connection definition object attributes */
        flom_tcp_set_socket_type(flom_conn_get_tcp(conn), SOCK_STREAM);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TCP_CONNECT_ERROR:
                break;
            case DAEMON_ERROR:
                break;
            case DAEMON_NOT_STARTED1:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case CONNECT_ERROR1:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case DAEMON_NOT_STARTED2:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_client_connect_tcp/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_discover_udp(flom_config_t *config,
                             flom_conn_t *conn, int start_daemon,
                             sa_family_t *family)
{
    enum Exception { GETADDRINFO_ERROR
                     , INVALID_AI_FAMILY1
                     , CONNECT_ERROR
                     , MSG_SERIALIZE_ERROR
                     , SETSOCKOPT_ERROR
                     , SENDTO_ERROR
                     , CONNECTION_REFUSED
                     , RECVFROM_ERROR
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , SET_SIN6_SCOPE_ID_ERROR
                     , CLIENT_CONNECT_TCP_ERROR
                     , INVALID_AI_FAMILY2
                     , CLIENT_DISCOVER_UDP_CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *result = NULL;
    int fd = FLOM_NULL_FD;
    struct flom_msg_s msg;
    
    FLOM_TRACE(("flom_client_discover_udp\n"));
    TRY {
        struct addrinfo hints;
        char port[100];
        int errcode;
        int found = FALSE;
        gint i;
        const struct addrinfo *gai = result;
        char out_buffer[FLOM_NETWORK_BUFFER_SIZE];
        char in_buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        ssize_t sent;
        struct timeval timeout;
        ssize_t received;
        struct sockaddr_storage from;
        socklen_t addrlen = sizeof(from);
        GMarkupParseContext *tmp_parser;
        
        flom_msg_init(&msg);
        
        FLOM_TRACE(("flom_client_discover_udp: using address '%s' "
                    "and port %d\n", flom_config_get_multicast_address(config),
                    flom_config_get_multicast_port(config)));
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        if (NULL != flom_config_get_network_interface(config))
            hints.ai_family = AF_INET6;
        else
            hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_multicast_port(config));

        if (0 != (errcode = getaddrinfo(
                      flom_config_get_multicast_address(config),
                      port, &hints, &result))) {
            FLOM_TRACE(("flom_client_discover_udp/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            int sock_opt = 1;
            u_char sock_opt2 = flom_config_get_discovery_ttl(config);
            u_int ipv6_iface = 0;
            int ipv6_hops = flom_config_get_discovery_ttl(config);
            
            FLOM_TRACE_ADDRINFO("flom_client_discover_udp/getaddrinfo(): ",
                                result);
            /* traverse the list and try to connect... */
            gai = result;
            while (NULL != gai && !found) {
                struct sockaddr_in local_addr4;
                struct sockaddr_in6 local_addr6;
                struct sockaddr *local_addr;
                socklen_t local_addr_len;
                int rc;

                /* prepare a local address on an ephemeral port to listen for
                   a reply from daemon */
                switch (gai->ai_family) {
                    case AF_INET:
                        memset(&local_addr4, 0, sizeof(local_addr4));
                        local_addr4.sin_family = gai->ai_family;
                        local_addr4.sin_addr.s_addr = htonl(INADDR_ANY);
                        local_addr4.sin_port = 0;
                        local_addr_len = sizeof(local_addr4);
                        local_addr = (struct sockaddr *)&local_addr4;
                        break;
                    case AF_INET6:
                        memset(&local_addr6, 0, sizeof(local_addr6));
                        local_addr6.sin6_family = gai->ai_family;
                        local_addr6.sin6_addr = in6addr_any;
                        local_addr6.sin6_port = 0;
                        local_addr_len = sizeof(local_addr6);
                        local_addr = (struct sockaddr *)&local_addr6;
                        break;
                    default:
                        FLOM_TRACE(("flom_client_discover_udp: "
                                    "gai->ai_family=%d\n", gai->ai_family));
                        THROW(INVALID_AI_FAMILY1);
                } /* switch (gai->ai_family) */
                FLOM_TRACE_HEX_DATA("flom_client_discover_udp: gai->ai_addr ",
                                    (void *)gai->ai_addr, gai->ai_addrlen);
                FLOM_TRACE_HEX_DATA("flom_client_discover_udp: local_addr ",
                                    (void *)local_addr, local_addr_len);
                if (-1 == (fd = socket(gai->ai_family, gai->ai_socktype,
                                       gai->ai_protocol))) {
                    FLOM_TRACE(("flom_client_discover_udp/socket(): "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                } else if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                            (void *)&sock_opt,
                                            sizeof(sock_opt))) {
                    FLOM_TRACE(("flom_client_discover_udp/setsockopt("
                                "SO_REUSEADDR) : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                    continue;
                }
                rc = AF_INET == gai->ai_family ?
                    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
                               (void *)local_addr, local_addr_len) :
                    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                               (void *)&ipv6_iface, sizeof(ipv6_iface));
                if (-1 == rc) {
                    FLOM_TRACE(("flom_client_discover_udp/setsockopt("
                                "IP_MULTICAST_IF/IPV6_MULTICAST_IF) : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                    continue;
                }
                rc = AF_INET == gai->ai_family ?
                    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL,
                               (void *)&sock_opt2, sizeof(sock_opt2)) :
                    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                               (void *)&ipv6_hops, sizeof(ipv6_hops));
                if (-1 == rc) {
                    FLOM_TRACE(("flom_client_discover_udp/setsockopt("
                                "IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS) : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                    continue;
                }
                if (-1 == bind(fd, local_addr, local_addr_len)) {
                    FLOM_TRACE(("flom_client_discover_udp/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                    continue;
                }
                found = TRUE;
                *family = gai->ai_family;
            } /* while (NULL != gai && !connected) */            
        }
        if (!found) {
            FLOM_TRACE(("flom_client_discover_udp: unable to use multicast "
                        "to discover a daemon\n"));
            THROW(CONNECT_ERROR);
        }
        /* prepare a discover message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_DISCOVER;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;
        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, out_buffer, sizeof(out_buffer),
                               &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* set time-out for receive operation */
        timeout.tv_sec = flom_config_get_discovery_timeout(config)/1000;
        timeout.tv_usec = (flom_config_get_discovery_timeout(
                               config)%1000)*1000;
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                             sizeof(timeout)))
            THROW(SETSOCKOPT_ERROR);
        
        /* attempt to locate the flom daemon */
        found = FALSE;
        memset(in_buffer, 0, sizeof(in_buffer));
        for (i=0; i<flom_config_get_discovery_attempts(config); ++i) {
            /* send discover message */
            FLOM_TRACE(("flom_client_discover_udp: sending discovery "
                        "message number %d...\n", i));
            if (flom_config_get_verbose(config))
                g_print("sending UDP multicast datagram to %s/%u ('%s')\n",
                        flom_config_get_multicast_address(config),
                        flom_config_get_multicast_port(config), out_buffer);
            if (to_send != (sent = sendto(fd, out_buffer, to_send, 0,
                                          gai->ai_addr, gai->ai_addrlen))) {
                FLOM_TRACE(("flom_client_discover_udp: sendto() to_send=%d, "
                            "sent=%d\n", to_send, sent));
                THROW(SENDTO_ERROR);
            }

            /* retrieve answer */
            FLOM_TRACE(("flom_client_discover_udp: waiting reply...\n"));
            memset(&from, 0, sizeof(from));
            if (-1 == (received = recvfrom(
                           fd, in_buffer, sizeof(in_buffer), 0,
                           (struct sockaddr *)&from, &addrlen))) {
                if (EAGAIN == errno || EWOULDBLOCK == errno) {
                    FLOM_TRACE(("flom_client_discover_udp: no answer from "
                                "UDP/IP multicast discovery\n"));
                    if (flom_config_get_verbose(config))
                        g_print("no reply from %s/%u\n",
                                flom_config_get_multicast_address(config),
                                flom_config_get_multicast_port(config));
                } else
                    THROW(RECVFROM_ERROR);
            } else {
                found = TRUE;
                if (flom_config_get_verbose(config))
                    g_print("reply from %s/%u is '%s'\n",
                            flom_config_get_multicast_address(config),
                            flom_config_get_multicast_port(config), in_buffer);
                break;
            }
        } /* for (i=0; i<flom_config_get_discovery_attempts(); ++i) */
        if (!found)
            THROW(CONNECTION_REFUSED);
        FLOM_TRACE(("flom_client_discover_udp: recvfrom()='%*.*s', %d\n",
                    received, received, in_buffer, received));        
        FLOM_TRACE_SOCKADDR("flom_client_discover_udp: from ",
                            (void *)&from, addrlen);
        /* this UDP/IP socket is no more useful */
        close(fd);
        fd = FLOM_NULL_FD;
        
        flom_msg_free(&msg);
        flom_msg_init(&msg);

        /* instantiate a new parser */
        if (NULL == (tmp_parser = g_markup_parse_context_new(
                         &flom_msg_parser, 0, (gpointer)&msg, NULL)))
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        flom_conn_set_parser(conn, tmp_parser);
        
        /* deserialize the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                               in_buffer, received, &msg,
                               flom_conn_get_parser(conn))))
            THROW(MSG_DESERIALIZE_ERROR);

        flom_msg_trace(&msg);        

        FLOM_TRACE(("flom_client_discover_udp: address=%p, "
                    "strlen(address)=%u\n",
                    msg.body.discover_16.network.address,
                    strlen(msg.body.discover_16.network.address)));

        /* if address length is > 0, server specified a callback address
           potentially different than UDP packet IP source */
        if (0 < strlen(msg.body.discover_16.network.address)) {
            /* update global configuration */
            flom_config_set_unicast_address(
                config, msg.body.discover_16.network.address);
            flom_config_set_unicast_port(
                config, msg.body.discover_16.network.port);
            /* IPv6 must save sin6_scope_id to reply using the right network
               interface */
            if (AF_INET6 == gai->ai_family) {
                struct sockaddr_in6 from6;
                memcpy(&from6, &from, addrlen = sizeof(from6));
                if (FLOM_RC_OK != (ret_cod = flom_config_set_sin6_scope_id(
                                       config, from6.sin6_scope_id)))
                    THROW(SET_SIN6_SCOPE_ID_ERROR);
            }   
            /* switch to normal TCP connect phase */
            if (FLOM_RC_OK != (ret_cod = flom_client_connect_tcp(
                                   config, conn, start_daemon)))
                THROW(CLIENT_CONNECT_TCP_ERROR);
        } else {
            struct sockaddr_in from4;
            struct sockaddr_in6 from6;
            struct sockaddr *sa;
            /* connect to discovered server using IP source address and
               port retrieved from message */
            switch (gai->ai_family) {
                case AF_INET:
                    memcpy(&from4, &from, addrlen = sizeof(from4));
                    from4.sin_port =
                        htons(msg.body.discover_16.network.port);
                    from4.sin_family = gai->ai_family;
                    sa = (struct sockaddr *)&from4;
                    break;
                case AF_INET6:
                    memcpy(&from6, &from, addrlen = sizeof(from6));
                    from6.sin6_port = htons(
                        msg.body.discover_16.network.port);
                    from6.sin6_family = gai->ai_family;
                    sa = (struct sockaddr *)&from6;
                    break;
                default:
                    FLOM_TRACE(("flom_client_discover_udp: "
                                "gai->ai_family=%d\n", gai->ai_family));
                    THROW(INVALID_AI_FAMILY2);
            } /* switch (gai->ai_family) */
            if (FLOM_RC_OK != (ret_cod = flom_client_discover_udp_connect(
                                   conn, sa, addrlen)))
                THROW(CLIENT_DISCOVER_UDP_CONNECT_ERROR);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case INVALID_AI_FAMILY1:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case SENDTO_ERROR:
                ret_cod = FLOM_RC_SENDTO_ERROR;
                break;
            case CONNECTION_REFUSED:
                ret_cod = FLOM_RC_CONNECTION_REFUSED;
                break;
            case RECVFROM_ERROR:
                ret_cod = FLOM_RC_RECVFROM_ERROR;
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case MSG_DESERIALIZE_ERROR:
            case SET_SIN6_SCOPE_ID_ERROR:
            case CLIENT_CONNECT_TCP_ERROR:
                break;
            case INVALID_AI_FAMILY2:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case CLIENT_DISCOVER_UDP_CONNECT_ERROR:
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
    flom_msg_free(&msg);
    FLOM_TRACE(("flom_client_discover_udp/excp=%d/"
                "ret_cod=%d/errno=%d ('%s')\n", excp, ret_cod, errno,
                strerror(errno)));
    return ret_cod;
}



int flom_client_discover_udp_connect(flom_conn_t *conn,
                                     const struct sockaddr *sa,
                                     socklen_t addrlen)
{
    enum Exception { SOCKET_ERROR
                     , CONNECT_ERROR
                     , SETSOCKOPT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int fd = FLOM_NULL_FD;
    
    FLOM_TRACE(("flom_client_discover_udp_connect\n"));
    TRY {
        int sock_opt = 1;

        if (-1 == (fd = socket(sa->sa_family, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        FLOM_TRACE_HEX_DATA("flom_client_discover_udp_connect: soin ",
                            (void *)sa, addrlen);
        if (-1 == connect(fd, sa, addrlen))
            THROW(CONNECT_ERROR);
        if (0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                            (void *)(&sock_opt), sizeof(sock_opt)))
            THROW(SETSOCKOPT_ERROR);
        /* set connection definition object attributes */
        flom_tcp_set_sockfd(flom_conn_get_tcp(conn), fd);
        flom_tcp_set_socket_type(flom_conn_get_tcp(conn), SOCK_STREAM);
        flom_tcp_set_sa_storage(flom_conn_get_tcp(conn),
                                (const struct sockaddr_storage *)sa, addrlen);
        /* avoid socket close operated by clean-up step */
        fd = FLOM_NULL_FD;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (FLOM_NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_client_discover_udp_connect/excp=%d/"
                "ret_cod=%d/errno=%d ('%s')\n", excp, ret_cod, errno,
                strerror(errno)));
    return ret_cod;
}


int flom_client_lock(flom_config_t *config, flom_conn_t *conn,
                     int timeout, char **element)
{
    enum Exception { NULL_OBJECT1
                     , G_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR1
                     , NETWORK_TIMEOUT1
                     , MSG_RETRIEVE_ERROR
                     , CONNECTION_CLOSED_BY_SERVER
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , MSG_DESERIALIZE_ERROR1
                     , PROTOCOL_LEVEL_MISMATCH
                     , MSG_DESERIALIZE_ERROR2
                     , PROTOCOL_ERROR1
                     , NO_TLS_CONNECTION
                     , NULL_OBJECT2
                     , TLS_CERT_CHECK_ERROR
                     , INTERNAL_ERROR
                     , NETWORK_TIMEOUT2
                     , CONNECT_WAIT_LOCK_ERROR
                     , LOCK_BUSY
                     , LOCK_IMPOSSIBLE
                     , LOCK_CANT_WAIT
                     , PROTOCOL_ERROR2
                     , MSG_FREE_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_msg_s msg;
    gchar *peer_name = NULL;
    
    FLOM_TRACE(("flom_client_lock\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        size_t to_read;
        GMarkupParseContext *tmp_parser;
        gchar *peerid = NULL;

        /* initialize message */
        flom_msg_init(&msg);
        /* prepare a request (lock) message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_LOCK;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;

        /* session */
        if (NULL == (msg.body.lock_8.session.peerid =
                     flom_tls_get_unique_id()))
            THROW(NULL_OBJECT1);
        /* resource */
        if (NULL == (msg.body.lock_8.resource.name =
                     g_strdup(flom_config_get_resource_name(config))))
            THROW(G_STRDUP_ERROR);
        msg.body.lock_8.resource.mode = flom_config_get_lock_mode(config);
        msg.body.lock_8.resource.wait =
            0 != flom_config_get_resource_timeout(config);
        msg.body.lock_8.resource.quantity =
            flom_config_get_resource_quantity(config);
        msg.body.lock_8.resource.create =
            flom_config_get_resource_create(config);
        msg.body.lock_8.resource.lifespan =
            flom_config_get_resource_idle_lifespan(config);

        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_conn_send(conn, buffer, to_send)))
            THROW(MSG_SEND_ERROR);
        flom_conn_set_last_step(conn, msg.header.pvs.step);

        flom_msg_trace(&msg);
        if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
            THROW(MSG_FREE_ERROR1);
        flom_msg_init(&msg);

        /* retrieve the reply message */
        ret_cod = flom_conn_recv(conn, buffer, sizeof(buffer), &to_read,
                                 timeout, NULL, NULL);
        switch (ret_cod) {
            case FLOM_RC_OK:
                break;
            case FLOM_RC_NETWORK_TIMEOUT:
                THROW(NETWORK_TIMEOUT1);
                break;
            default:
                THROW(MSG_RETRIEVE_ERROR);
        } /* switch (ret_cod) */

        /* an empty response is the result of connection closing on the
           server side */
        if (0 == to_read) {
            FLOM_TRACE(("flom_client_lock: flom daemon has closed the "
                        "connection, maybe a wrong request was sent...\n",
                        to_read));
            THROW(CONNECTION_CLOSED_BY_SERVER);
        }
        
        /* instantiate a new parser */
        if (NULL == (tmp_parser = g_markup_parse_context_new(
                         &flom_msg_parser, 0, (gpointer)&msg, NULL)))
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        flom_conn_set_parser(conn, tmp_parser);
        
        /* deserialize the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                               buffer, to_read, &msg,
                               flom_conn_get_parser(conn))))
            THROW(MSG_DESERIALIZE_ERROR1);
        flom_conn_set_last_step(conn, msg.header.pvs.step);
        /* check the parser completed without errors */
        if (FLOM_MSG_STATE_READY != msg.state) {
            /* check message level */
            if (FLOM_MSG_LEVEL != msg.header.level) {
                THROW(PROTOCOL_LEVEL_MISMATCH);
            } else {
                THROW(MSG_DESERIALIZE_ERROR2);
            }
        } /* if (FLOM_MSG_STATE_READY != msg->state) */
        
        flom_msg_trace(&msg);

        /* check lock answer */
        if (FLOM_MSG_VERB_LOCK != msg.header.pvs.verb ||
            2*FLOM_MSG_STEP_INCR != msg.header.pvs.step)
            THROW(PROTOCOL_ERROR1);
        /* retrieve peer id */
        if (NULL != (peerid = flom_msg_get_peerid(&msg))) {
            FLOM_TRACE(("flom_client_lock: remote peer is presenting "
                        "itself with id='%s'\n", peerid));
            syslog(LOG_INFO, FLOM_SYSLOG_FLM016I, peerid,
                   msg.header.pvs.verb, msg.header.pvs.step);
        }
        
        /* check peer id if requested */
        if (flom_config_get_tls_check_peer_id(config)) {
            flom_tls_t *tls = NULL;
            /* check it's a TLS connection; if not, maybe an internal
               error */
            if (NULL == (tls = flom_conn_get_tls(conn)))
                THROW(NO_TLS_CONNECTION);
            if (NULL == (peer_name = flom_tcp_retrieve_peer_name(
                             flom_conn_get_tcp(conn))))
                THROW(NULL_OBJECT2);
            if (FLOM_RC_OK != (ret_cod = flom_tls_cert_check(
                                   tls, peerid, peer_name))) {
                THROW(TLS_CERT_CHECK_ERROR);
            }
        } /* if (flom_config_get_tls_check_peer_id(config)) */

        switch (msg.body.lock_16.answer.rc) {
            case FLOM_RC_OK:
                /* copy element if available */
                if (NULL != msg.body.lock_16.answer.element) {
                    FLOM_TRACE(("flom_client_lock: element=%p, *element=%p\n",
                                element, *element));
                    g_free(*element);
                    FLOM_TRACE(("flom_client_lock: element=%p, *element=%p\n",
                                element, *element));
                    *element = g_strdup(msg.body.lock_16.answer.element);
                    FLOM_TRACE(("flom_client_lock: element=%p, *element=%p\n",
                                element, *element));
                }
                break;
            case FLOM_RC_LOCK_ENQUEUED:
            case FLOM_RC_LOCK_WAIT_RESOURCE:
                FLOM_TRACE(("flom_client_lock: lock can not be acquired now "
                            "(%s), waiting...\n",
                            flom_strerror(msg.body.lock_16.answer.rc)));
                ret_cod = flom_client_wait_lock(conn, &msg, timeout);
                switch (ret_cod) {
                    case FLOM_RC_OK:
                        /* copy element if available */
                        if (3*FLOM_MSG_STEP_INCR ==
                            flom_conn_get_last_step(conn)) {
                            if (NULL != msg.body.lock_24.answer.element) {
                                g_free(*element);
                                *element = g_strdup(
                                    msg.body.lock_24.answer.element);
                            }
                        } else if (4*FLOM_MSG_STEP_INCR ==
                                   flom_conn_get_last_step(conn)) {
                            if (NULL != msg.body.lock_24.answer.element) {
                                g_free(*element);
                                *element = g_strdup(
                                    msg.body.lock_24.answer.element);
                            }
                        } else
                            THROW(INTERNAL_ERROR);
                        break;
                    case FLOM_RC_NETWORK_TIMEOUT:
                        THROW(NETWORK_TIMEOUT2);
                        break;
                    default:
                        THROW(CONNECT_WAIT_LOCK_ERROR);
                } /* switch (ret_cod) */
                break;
            case FLOM_RC_LOCK_BUSY:
                ret_cod = msg.body.lock_16.answer.rc;
                THROW(LOCK_BUSY);
                break;
            case FLOM_RC_LOCK_IMPOSSIBLE:
                ret_cod = msg.body.lock_16.answer.rc;
                THROW(LOCK_IMPOSSIBLE);
                break;
            case FLOM_RC_LOCK_CANT_WAIT:
                ret_cod = msg.body.lock_16.answer.rc;
                THROW(LOCK_CANT_WAIT);
                break;                
            default:
                THROW(PROTOCOL_ERROR2);
                break;
        } /* switch (msg.body.lock_16.answer.rc) */

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR1:
            case NETWORK_TIMEOUT1:
            case MSG_RETRIEVE_ERROR:
                break;
            case CONNECTION_CLOSED_BY_SERVER:
                ret_cod = FLOM_RC_CONNECTION_CLOSED_BY_SERVER;
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case MSG_DESERIALIZE_ERROR1:
                ret_cod = FLOM_RC_MSG_DESERIALIZE_ERROR;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case PROTOCOL_LEVEL_MISMATCH:
                ret_cod = FLOM_RC_PROTOCOL_LEVEL_MISMATCH;
                break;
            case MSG_DESERIALIZE_ERROR2:
                ret_cod = FLOM_RC_MSG_DESERIALIZE_ERROR;
                break;
            case PROTOCOL_ERROR1:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NO_TLS_CONNECTION:
                ret_cod = FLOM_RC_NO_TLS_CONNECTION;
                break;
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case TLS_CERT_CHECK_ERROR:
            case NETWORK_TIMEOUT2:
                break;                
            case PROTOCOL_ERROR2:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case CONNECT_WAIT_LOCK_ERROR:
            case LOCK_BUSY:
            case LOCK_IMPOSSIBLE:
            case LOCK_CANT_WAIT:
            case MSG_FREE_ERROR2:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release peer_name if necessary */
    if (NULL != peer_name)
        g_free(peer_name);
    /* release markup parser */
    flom_conn_free_parser(conn);
    flom_msg_free(&msg);        
    
    FLOM_TRACE(("flom_client_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_wait_lock(flom_conn_t *conn,
                          struct flom_msg_s *msg, int timeout)
{
    enum Exception { NETWORK_TIMEOUT
                     , MSG_RETRIEVE_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , PROTOCOL_ERROR
                     , LOCK_CANT_LOCK
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_wait_lock\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_read;
        struct flom_msg_body_answer_s mba;

        /* more than one answer can arrive */
        while (TRUE) {
            /* retrieve the reply message */
            ret_cod = flom_conn_recv(
                conn, buffer, sizeof(buffer), &to_read, timeout, NULL, NULL);
            switch (ret_cod) {
                case FLOM_RC_OK:
                    break;
                case FLOM_RC_NETWORK_TIMEOUT:
                    THROW(NETWORK_TIMEOUT);
                    break;
                default:
                    THROW(MSG_RETRIEVE_ERROR);
            } /* switch (ret_cod) */
            
            flom_msg_free(msg);
            flom_msg_init(msg);
            
            /* deserialize the reply message */
            if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                   buffer, to_read, msg,
                                   flom_conn_get_parser(conn))))
                THROW(MSG_DESERIALIZE_ERROR);
            flom_conn_set_last_step(conn, msg->header.pvs.step);
            flom_msg_trace(msg);

            /* is arriving an intermediate message (resource does not exist
               condition)? */
            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb) {
                switch (msg->header.pvs.step) {
                    case 2*FLOM_MSG_STEP_INCR:
                        mba = msg->body.lock_16.answer;
                        break;
                    case 3*FLOM_MSG_STEP_INCR:
                        mba = msg->body.lock_24.answer;
                        break;
                    case 4*FLOM_MSG_STEP_INCR:
                        /* last step, MUST be OK */
                        mba = msg->body.lock_32.answer;
                        if (FLOM_RC_OK != mba.rc) {
                            FLOM_TRACE(("flom_client_wait_lock: lock can "
                                        "NOT be acquired, leaving...\n"));
                            THROW(LOCK_CANT_LOCK);
                        } /* if (FLOM_RC_OK != mba.rc) */
                        break;
                    default:
                        THROW(PROTOCOL_ERROR);
                        break;
                } /* switch (msg->header.pvs.step) */
            } /* if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb) */
            
            if (FLOM_RC_LOCK_ENQUEUED == mba.rc) {
                FLOM_TRACE(("flom_client_wait_lock: the resource is "
                            "busy, looping again...\n"));
                continue;
            } /* if (FLOM_RC_LOCK_ENQUEUED == mba.rc */
            /* last message was arrived, leaving the loop */
            break;
        } /* while (TRUE) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NETWORK_TIMEOUT:
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:               
                break;
            case PROTOCOL_ERROR:
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
    FLOM_TRACE(("flom_client_wait_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_unlock(flom_config_t *config, flom_conn_t *conn,
                       int rollback)
{
    enum Exception { G_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_unlock\n"));
    TRY {
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;

        /* prepare a request (lock) message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_UNLOCK;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;

        if (NULL == (msg.body.unlock_8.resource.name =
                     g_strdup(flom_config_get_resource_name(config))))
            THROW(G_STRDUP_ERROR);
        msg.body.unlock_8.resource.rollback = rollback;

        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_conn_send(conn, buffer, to_send)))
            THROW(MSG_SEND_ERROR);
        flom_conn_set_last_step(conn, msg.header.pvs.step);
        
        if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
            THROW(MSG_FREE_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_client_unlock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_disconnect(flom_conn_t *conn)
{
    enum Exception { CONN_TERMINATE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_disconnect\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        /* gracely shutdown write half socket */
        if (-1 == shutdown(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                           SHUT_WR))
            FLOM_TRACE(("flom_client_disconnect/shutdown(%d,SHUT_WR)=%d "
                        "('%s')\n",
                        flom_tcp_get_sockfd(flom_conn_get_tcp(conn)), errno,
                        strerror(errno)));
        /* pick-up socket close/error but does not wait */
        if (-1 == recv(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)), buffer,
                       sizeof(buffer), 0 /* MSG_DONTWAIT */)) 
            FLOM_TRACE(("flom_client_disconnect/recv(%d,,%u,0)=%d "
                        "('%s')\n",
                        flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                        sizeof(buffer), errno, strerror(errno)));
        /* shutdown read half socket */
        if (-1 == shutdown(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                           SHUT_RD))
            FLOM_TRACE(("flom_client_disconnect/shutdown(%d,SHUT_RD)=%d "
                        "('%s')\n",
                        flom_tcp_get_sockfd(flom_conn_get_tcp(conn)), errno,
                        strerror(errno)));
        /* connection termination */
        if (FLOM_RC_OK != (ret_cod = flom_conn_terminate(conn)))
            THROW(CONN_TERMINATE_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONN_TERMINATE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_client_disconnect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_shutdown(flom_config_t *config, int immediate)
{
    enum Exception { NULL_OBJECT1
                     , DAEMON_NOT_STARTED
                     , CLIENT_CONNECT_ERROR
                     , NULL_OBJECT2
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , CLIENT_DISCONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    flom_conn_t *conn;
    struct flom_msg_s msg;
    
    FLOM_TRACE(("flom_client_shutdown\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send, received;

        /* creating a new connection object */
        if (NULL == (conn = flom_conn_new(config)))
            THROW(NULL_OBJECT1);
        
        /* initializing */
        flom_msg_init(&msg);

        /* connect to daemon */
        ret_cod = flom_client_connect(config, conn, FALSE);
        switch (ret_cod) {
            case FLOM_RC_OK:
                FLOM_TRACE(("flom_client_shutdown: connection with daemon "
                            "obtained...\n"));
                break;
            case FLOM_RC_DAEMON_NOT_STARTED:
                FLOM_TRACE(("flom_client_shutdown: the daemon is not "
                            "running and cannot be stopped\n"));
                THROW(DAEMON_NOT_STARTED);
                break;
            default:
                THROW(CLIENT_CONNECT_ERROR);
        } /* switch (ret_cod) */

        /* prepare a shutdown message */
        /* header values */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_MNGMNT;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;
        /* body values */
        /* session */
        if (NULL == (msg.body.mngmnt_8.session.peerid =
                     flom_tls_get_unique_id()))
            THROW(NULL_OBJECT2);
        /* action */
        msg.body.mngmnt_8.action = FLOM_MSG_MNGMNT_ACTION_SHUTDOWN;
        msg.body.mngmnt_8.action_data.shutdown.immediate = immediate;
        
        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_conn_send(conn, buffer, to_send)))
            THROW(MSG_SEND_ERROR);
        flom_conn_set_last_step(conn, msg.header.pvs.step);
        FLOM_TRACE(("flom_client_shutdown: shutdown message sent "
                    "to daemon...\n"));

        /* wait 1000ms reply from the daemon, typically an error */
        ret_cod = flom_conn_recv(conn, buffer, sizeof(buffer), &received,
                                 1000, NULL, NULL);
        FLOM_TRACE(("flom_client_shutdown/flom_conn_recv: ret_cod=%d "
                    "'%s'\n", ret_cod, flom_strerror(ret_cod)));
        if (FLOM_RC_RECV_ERROR != ret_cod)
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM020N, ret_cod,
                   flom_strerror(ret_cod));
        
        /* gracefully disconnect from daemon */
        ret_cod = flom_client_disconnect(conn);
        switch (ret_cod) {
            case FLOM_RC_OK:
                FLOM_TRACE(("flom_client_shutdown: disconnected "
                            "from daemon\n"));
                break;
            default:
                THROW(CLIENT_DISCONNECT_ERROR);
        } /* switch (ret_cod) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case DAEMON_NOT_STARTED:
            case CLIENT_CONNECT_ERROR:
                break;
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case CLIENT_DISCONNECT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release connection object */
    flom_conn_delete(conn);
    
    /* release msg dynamically allocated memory */
    flom_msg_free(&msg);
    
    FLOM_TRACE(("flom_client_shutdown/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



