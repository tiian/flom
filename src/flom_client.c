/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CLIENT



int flom_client_connect(struct flom_conn_data_s *cd)
{
    enum Exception { CLIENT_CONNECT_LOCAL_ERROR
                     , CLIENT_CONNECT_TCP_ERROR
                     , CLIENT_DISCOVER_UDP_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_connect\n"));
    TRY {
        /* reset connection data struct */
        memset(cd, 0, sizeof(cd));

        /* choose and instantiate connection type */
        if (NULL != flom_config_get_socket_name()) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect_local(cd)))
                THROW(CLIENT_CONNECT_LOCAL_ERROR);
        } else if (NULL != flom_config_get_unicast_address()) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect_tcp(cd)))
                THROW(CLIENT_CONNECT_TCP_ERROR);
        } else if (NULL != flom_config_get_multicast_address()) {
            if (FLOM_RC_OK != (ret_cod = flom_client_discover_udp(cd)))
                THROW(CLIENT_DISCOVER_UDP_ERROR);
        } else /* this condition must be impossible! */
            THROW(INTERNAL_ERROR);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CLIENT_CONNECT_LOCAL_ERROR:
            case CLIENT_CONNECT_TCP_ERROR:
            case CLIENT_DISCOVER_UDP_ERROR:
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
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



int flom_client_connect_local(struct flom_conn_data_s *cd)
{
    enum Exception { SOCKET_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED 
                     , CONNECT_ERROR1
                     , CONNECT_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_connect_local\n"));
    TRY {
        FLOM_TRACE(("flom_client_connect_local: connecting to socket '%s'\n",
                    flom_config_get_socket_name()));

        if (-1 == (cd->fd = socket(AF_LOCAL, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        cd->saun.sun_family = AF_LOCAL;
        strcpy(cd->saun.sun_path, global_config.socket_name);
        cd->addr_len = sizeof(cd->saun);
        if (-1 == connect(cd->fd, (struct sockaddr *)&cd->saun,
                          cd->addr_len)) {
            if (ENOENT == errno || ECONNREFUSED == errno) {
                if (0 != flom_config_get_lifespan()) {
                    FLOM_TRACE(("flom_client_connect_local: connection "
                                "failed, activating a new daemon\n"));
                    /* daemon is not active, starting it... */
                    if (FLOM_RC_OK != (ret_cod = flom_daemon(
                                           cd->saun.sun_family)))
                        THROW(DAEMON_ERROR);
                    /* trying to connect again... */
                    if (-1 == connect(cd->fd, (struct sockaddr *)&cd->saun,
                                      cd->addr_len))
                        THROW(DAEMON_NOT_STARTED);
                    FLOM_TRACE(("flom_client_connect_local: connected to "
                                "flom daemon\n"));
                } else {
                    FLOM_TRACE(("flom_client_connect_local: connection "
                                "failed, a new daemon can not be started "
                                "because daemon lifespan is 0\n"));
                    THROW(CONNECT_ERROR1);
                }
            } else {
                THROW(CONNECT_ERROR2);
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
            case CONNECT_ERROR1:
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



int flom_client_connect_tcp(struct flom_conn_data_s *cd)
{
    enum Exception { GETADDRINFO_ERROR
                     , DAEMON_ERROR
                     , DAEMON_NOT_STARTED
                     , CONNECT_ERROR1
                     , SETSOCKOPT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    struct addrinfo *result = NULL;
    int fd = NULL_FD;
    
    FLOM_TRACE(("flom_client_connect_tcp\n"));
    TRY {
        struct addrinfo hints;
        int errcode, sock_opt = 1;
        const struct addrinfo *p = NULL;
        char port[100];
        
        FLOM_TRACE(("flom_client_connect_tcp: connecting to address '%s' "
                    "and port %d\n", flom_config_get_unicast_address(),
                    flom_config_get_unicast_port()));
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME;
        /* remove this filter to support IPV6, but most of the following
           calls must be fixed! */
        hints.ai_family = AF_INET; 
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(port, sizeof(port), "%u", flom_config_get_unicast_port());

        if (0 != (errcode = getaddrinfo(flom_config_get_unicast_address(),
                                        port, &hints, &result))) {
            FLOM_TRACE(("flom_client_connect_tcp/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            FLOM_TRACE_ADDRINFO("flom_client_connect_tcp/getaddrinfo(): ",
                                result);
            p = flom_client_connect_tcp_try(result, &fd);
            if (NULL == p) {
                if (0 != flom_config_get_lifespan()) {
                    FLOM_TRACE(("flom_client_connect_tcp: connection "
                                "failed, activating a new daemon\n"));
                    /* daemon is not active, starting it... */
                    if (FLOM_RC_OK != (ret_cod = flom_daemon(hints.ai_family)))
                        THROW(DAEMON_ERROR);
                    /* trying to connect again... */
                    p = flom_client_connect_tcp_try(result, &fd);
                    if (NULL == p)
                        THROW(DAEMON_NOT_STARTED);
                } else {
                    FLOM_TRACE(("flom_client_connect_tcp: connection "
                                "failed, a new daemon can not be started "
                                "because daemon lifespan is 0\n"));
                    THROW(CONNECT_ERROR1);
                }
            }
        } /* if (0 != (errcode = getaddrinfo( */
        if (0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                            (void *)(&sock_opt), sizeof(sock_opt)))
            THROW(SETSOCKOPT_ERROR);
        /* set connection definition object attributes */
        cd->fd = fd;
        memcpy(&cd->sain, &p->ai_addr, p->ai_addrlen);
        cd->addr_len = p->ai_addrlen;

        /* avoid socket close operated by clean-up step */
        fd = NULL_FD;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case DAEMON_ERROR:
                break;
            case DAEMON_NOT_STARTED:
                ret_cod = FLOM_RC_DAEMON_NOT_STARTED;
                break;
            case CONNECT_ERROR1:
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
    if (NULL != result)
        freeaddrinfo(result);
    if (NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_client_connect_tcp/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



const struct addrinfo *flom_client_connect_tcp_try(
    const struct addrinfo *gai, int *fd)
{
    const struct addrinfo *found = NULL; 
    *fd = NULL_FD;
    /* traverse the list and try to connect... */
    while (NULL != gai && NULL == found) {
        if (-1 == (*fd = socket(gai->ai_family, gai->ai_socktype,
                                gai->ai_protocol))) {
            FLOM_TRACE(("flom_client_connect_tcp_try/socket(): "
                        "errno=%d '%s', skipping...\n", errno,
                        strerror(errno)));
            gai = gai->ai_next;
        } else {
            if (-1 == connect(*fd, gai->ai_addr, gai->ai_addrlen)) {
                FLOM_TRACE(("flom_client_connect_tcp_try/connect() : "
                            "errno=%d '%s', skipping...\n", errno,
                            strerror(errno)));
                gai = gai->ai_next;
                close(*fd);
                *fd = NULL_FD;
            } else
                found = gai;
        } /* if (-1 == (*fd = socket( */
    } /* while (NULL != gai && !connected) */
    return found;
}



int flom_client_discover_udp(struct flom_conn_data_s *cd)
{
    enum Exception { GETADDRINFO_ERROR
                     , CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *result = NULL;
    int fd = NULL_FD;
    
    FLOM_TRACE(("flom_client_discover_udp\n"));
    TRY {
        struct addrinfo hints;
        char port[100];
        int errcode;
        int found = FALSE;
        const struct addrinfo *gai = result;
        
        FLOM_TRACE(("flom_client_discover_udp: using address '%s' "
                    "and port %d\n", flom_config_get_multicast_address(),
                    flom_config_get_multicast_port()));
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME;
        /* remove this filter to support IPV6, but most of the following
           calls must be fixed! */
        hints.ai_family = AF_INET; 
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        snprintf(port, sizeof(port), "%u", flom_config_get_multicast_port());

        if (0 != (errcode = getaddrinfo(flom_config_get_multicast_address(),
                                        port, &hints, &result))) {
            FLOM_TRACE(("flom_client_discover_udp/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            int sock_opt = 1;
            
            FLOM_TRACE_ADDRINFO("flom_client_discover_udp/getaddrinfo(): ",
                                result);
            /* traverse the list and try to connect... */
            gai = result;
            while (NULL != gai && !found) {
                u_char loop = 0;
                FLOM_TRACE_HEX_DATA("flom_client_discover_udp: ai_addr ",
                                    (void *)gai->ai_addr, gai->ai_addrlen);
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
                    fd = NULL_FD;
                } else if (-1 == bind(fd, gai->ai_addr, gai->ai_addrlen)) {
                    FLOM_TRACE(("flom_client_discover_udp/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = NULL_FD;
                } else { /* switching to multicast mode */
                    struct ip_mreq mreq;
                    
                    memcpy(&mreq.imr_multiaddr,
                           &((struct sockaddr_in *)gai->ai_addr)->sin_addr,
                           sizeof(struct in_addr));
                    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                    if (-1 == setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                            &mreq, sizeof(mreq))) {
                        FLOM_TRACE(("flom_client_discover_udp/setsockopt("
                                    "IP_ADD_MEMBERSHIP) : "
                                    "errno=%d '%s', skipping...\n", errno,
                                    strerror(errno)));
                        gai = gai->ai_next;
                        close(fd);
                        fd = NULL_FD;
                    } else if (-1 == setsockopt( /* disable loopback */
                                   fd, IPPROTO_IP, IP_MULTICAST_LOOP,
                                   &loop, sizeof(loop))) {
                        FLOM_TRACE(("flom_client_discover_udp/setsockopt("
                                    "IP_MULTICAST_LOOP) : "
                                    "errno=%d '%s', skipping...\n", errno,
                                    strerror(errno)));
                        gai = gai->ai_next;
                        close(fd);
                        fd = NULL_FD;
                    } else {
                        found = TRUE;
                    }  /* else */
                } /* if (-1 == (*fd = socket( */
            } /* while (NULL != gai && !connected) */            
        }
        if (!found) {
            FLOM_TRACE(("flom_client_discover_udp: unable to use multicast "
                        "to discover a daemon\n"));
            THROW(CONNECT_ERROR);
        }
        /* send a packet */
    {
        ssize_t sent, received;
        char *packet = "This is a try!";
        char buffer[200];
        struct sockaddr from;
        socklen_t addrlen;
        int new_fd;

        /* @@@ */
        new_fd = socket(AF_INET, SOCK_DGRAM, 0);
        FLOM_TRACE(("flom_client_discover_udp: new_fd=%d\n", new_fd));
        
        sent = sendto(new_fd, packet, strlen(packet), 0,
                      gai->ai_addr, gai->ai_addrlen);
        FLOM_TRACE(("flom_client_discover_udp: sendto('%s')=%d\n",
                    packet, sent));

        memset(&from, 0, sizeof(from));
        received = recvfrom(fd, buffer, sizeof(buffer), 0, &from, &addrlen);
        FLOM_TRACE(("flom_client_discover_udp: recvfrom()='%*.*s', %d\n",
                    received, received, buffer, received));
        
        FLOM_TRACE_HEX_DATA("flom_client_discover_udp: from ",
                            (void *)&from, addrlen);
        
    }
        
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
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
    FLOM_TRACE(("flom_client_discover_udp/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_lock(struct flom_conn_data_s *cd, int timeout)
{
    enum Exception { G_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NETWORK_TIMEOUT1
                     , MSG_RETRIEVE_ERROR
                     , G_MARKUP_PARSE_CONTEXT_NEW_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , PROTOCOL_ERROR1
                     , NETWORK_TIMEOUT2
                     , CONNECT_WAIT_LOCK_ERROR
                     , LOCK_BUSY
                     , PROTOCOL_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_lock\n"));
    TRY {
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        ssize_t to_read;

        /* prepare a request (lock) message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_LOCK;
        msg.header.pvs.step = FLOM_MSG_STEP_INCR;

        if (NULL == (msg.body.lock_8.resource.name =
                     g_strdup(global_config.resource_name)))
            THROW(G_STRDUP_ERROR);
        msg.body.lock_8.resource.mode = flom_config_get_lock_mode();
        msg.body.lock_8.resource.wait = flom_config_get_resource_wait();

        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_send(
                               cd->fd, buffer, to_send)))
            THROW(MSG_SEND_ERROR);

        if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
            THROW(MSG_FREE_ERROR);
        flom_msg_init(&msg);

        /* retrieve the reply message */
        ret_cod = flom_msg_retrieve(
            cd->fd, buffer, sizeof(buffer), &to_read, timeout);
        switch (ret_cod) {
            case FLOM_RC_OK:
                break;
            case FLOM_RC_NETWORK_TIMEOUT:
                THROW(NETWORK_TIMEOUT1);
                break;
            default:
                THROW(MSG_RETRIEVE_ERROR);
        } /* switch (ret_cod) */

        /* instantiate a new parser */
        if (NULL == (cd->gmpc = g_markup_parse_context_new(
                         &flom_msg_parser, 0, (gpointer)&msg, NULL)))
            THROW(G_MARKUP_PARSE_CONTEXT_NEW_ERROR);
        
        /* deserialize the reply message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                               buffer, to_read, &msg, cd->gmpc)))
            THROW(MSG_DESERIALIZE_ERROR);

        flom_msg_trace(&msg);

        /* check lock answer */
        if (FLOM_MSG_VERB_LOCK != msg.header.pvs.verb ||
            2*FLOM_MSG_STEP_INCR != msg.header.pvs.step)
            THROW(PROTOCOL_ERROR1);
        switch (msg.body.lock_16.answer.rc) {
            case FLOM_RC_OK:
                break;
            case FLOM_RC_LOCK_ENQUEUED:
                FLOM_TRACE(("flom_client_lock: resource is busy, "
                            "waiting...\n"));
                ret_cod = flom_client_wait_lock(cd, &msg, timeout);
                switch (ret_cod) {
                    case FLOM_RC_OK:
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
            default:
                THROW(PROTOCOL_ERROR2);
                break;
        } /* switch (msg.body.lock_16.answer.rc) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
            case NETWORK_TIMEOUT1:
            case MSG_RETRIEVE_ERROR:
                break;
            case G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;
                break;
            case MSG_DESERIALIZE_ERROR:
                break;
            case PROTOCOL_ERROR1:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NETWORK_TIMEOUT2:
                break;                
            case PROTOCOL_ERROR2:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case CONNECT_WAIT_LOCK_ERROR:
            case LOCK_BUSY:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */

    /* release markup parser */
    if (NULL != cd->gmpc) {
        g_markup_parse_context_free(cd->gmpc);
        cd->gmpc = NULL;
    }
    
    FLOM_TRACE(("flom_client_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_client_wait_lock(struct flom_conn_data_s *cd,
                          struct flom_msg_s *msg, int timeout)
{
    enum Exception { NETWORK_TIMEOUT
                     , MSG_RETRIEVE_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , PROTOCOL_ERROR1
                     , LOCK_CANT_LOCK
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_wait_lock\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        ssize_t to_read;
        
        /* retrieve the reply message */
        ret_cod = flom_msg_retrieve(
            cd->fd, buffer, sizeof(buffer), &to_read, timeout);
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
                               buffer, to_read, msg, cd->gmpc)))
            THROW(MSG_DESERIALIZE_ERROR);

        flom_msg_trace(msg);
        
        /* check lock answer */
        if (FLOM_MSG_VERB_LOCK != msg->header.pvs.verb ||
            3*FLOM_MSG_STEP_INCR != msg->header.pvs.step)
            THROW(PROTOCOL_ERROR1);
        if (FLOM_RC_OK != msg->body.lock_24.answer.rc) {
            FLOM_TRACE(("flom_client_wait_lock: lock can NOT be acquired, "
                        "leaving...\n"));
            THROW(LOCK_CANT_LOCK);
        }   
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NETWORK_TIMEOUT:
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:               
                break;
            case PROTOCOL_ERROR1:
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



int flom_client_unlock(struct flom_conn_data_s *cd)
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

        if (NULL == (msg.body.lock_8.resource.name =
                     g_strdup(global_config.resource_name)))
            THROW(G_STRDUP_ERROR);

        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);

        /* send the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_send(
                               cd->fd, buffer, to_send)))
            THROW(MSG_SEND_ERROR);

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



int flom_client_disconnect(struct flom_conn_data_s *cd)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_client_disconnect\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        /* gracely shutdown write half socket */
        if (-1 == shutdown(cd->fd, SHUT_WR))
            FLOM_TRACE(("flom_client_disconnect/shutdown(%d,SHUT_WR)=%d "
                        "('%s')\n", cd->fd, errno, strerror(errno)));
        /* pick-up socket close/error but does not wait */
        if (-1 == recv(cd->fd, buffer, sizeof(buffer), 0 /* MSG_DONTWAIT */)) 
            FLOM_TRACE(("flom_client_disconnect/recv(%d,,%u,MSG_DONTWAIT)=%d "
                        "('%s')\n", cd->fd, sizeof(buffer),
                        errno, strerror(errno)));
        /* shutdown read half socket */
        if (-1 == shutdown(cd->fd, SHUT_RD))
            FLOM_TRACE(("flom_client_disconnect/shutdown(%d,SHUT_RD)=%d "
                        "('%s')\n", cd->fd, errno, strerror(errno)));
        cd->fd = NULL_FD;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
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

