/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_CONN_H
# define FLOM_CONN_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_msg.h"
#include "flom_tcp.h"
#include "flom_tls.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_CONNS



/**
 * Possible state of a connection
 */
typedef enum flom_conn_state_e {
    /**
     * The connection is managed by main daemon thread (the listener thread)
     */
    FLOM_CONN_STATE_DAEMON,
    /**
     * The connection is managed by a locker thread (the resource manager
     * threads)
     */
    FLOM_CONN_STATE_LOCKER,
    /**
     * The connection is no more useful and must be eliminated by
     * @ref flom_conns_clean
     */
    FLOM_CONN_STATE_REMOVE
} flom_conn_state_t;



/**
 * Class of objects used to store connection data
 */
typedef struct {
    /**
     * Connection state
     */
    flom_conn_state_t     state;
    /**
     * The connection is related to a client waiting for resource creation
     * (it's logically parked inside a virtual incubator); this is a boolean
     * property
     */
    int                   wait;
    /**
     * Step of the last sent/received  message
     */
    int                   last_step;
    /**
     * TCP/IP connection data
     */
    flom_tcp_t            tcp;
    /**
     * TLS/SSL connection data
     */
    flom_tls_t           *tls;
    /**
     * Last received message (allocated by malloc)
     */
    struct flom_msg_s    *msg;
    /**
     * GMarkup Parser context (allocated by g_markup_parse_context_new)
     */
    GMarkupParseContext  *parser;
} flom_conn_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Factory method to create a new flom_conn_t object
     * @param config IN configuration object reference or NULL
     * @return a new object or NULL if any error occurred
     */
    flom_conn_t *flom_conn_new(flom_config_t *config);
    


    /**
     * Destroy an object of type flom_conn_t
     * @param obj IN object to be destroyed
     */
    void flom_conn_delete(flom_conn_t *obj);



    /**
     * Initialize a connection object with network parameters
     * @param obj IN connection object
     * @param domain IN communication domain (AF_UNIX/AF_LOCAL, AF_INET,
     *                  AF_INET6)
     * @param sockfd IN TCP socket file descriptor
     * @param type IN TCP socket type
     * @param addrlen IN address lenght
     * @param sa IN socket address
     * @param main_thread IN boolean value: TRUE for daemon main thread
     *                       (father/listener), FALSE for locker child thread
     *                       (child/locker)
     * @return a reason code
     */
    int flom_conn_init(flom_conn_t *obj, int domain, int sockfd, int type,
                       socklen_t addrlen, const struct sockaddr *sa,
                       int main_thread);

    
    
    /**
     * Getter method for state property
     * @param obj IN connection object
     * @return state
     */
    static inline int flom_conn_get_state(const flom_conn_t *obj) {
        return obj->state;
    }
    
    
    
    /**
     * Setter method for state property
     * @param obj IN/OUT connection object
     * @param value IN new value for state
     */
    static inline void flom_conn_set_state(flom_conn_t *obj, int value) {
        obj->state = value;
    }


    
    /**
     * Getter method for wait property
     * @param obj IN connection object
     * @return wait
     */
    static inline int flom_conn_get_wait(const flom_conn_t *obj) {
        return obj->wait;
    }
    
    
    
    /**
     * Setter method for wait property
     * @param obj IN/OUT connection object
     * @param value IN new value for wait
     */
    static inline void flom_conn_set_wait(flom_conn_t *obj, int value) {
        obj->wait = value;
    }


    
    /**
     * Getter method for last_step property
     * @param obj IN connection object
     * @return last_step
     */
    static inline int flom_conn_get_last_step(const flom_conn_t *obj) {
        return obj->last_step;
    }
    
    
    
    /**
     * Setter method for last_step property
     * @param obj IN/OUT connection object
     * @param value IN new value for last_step
     */
    static inline void flom_conn_set_last_step(flom_conn_t *obj, int value) {
        obj->last_step = value;
    }


    
    /**
     * Getter method for tcp property
     * @param obj IN connection object
     * @return tcp
     */
    static inline flom_tcp_t *flom_conn_get_tcp(flom_conn_t *obj) {
        return &obj->tcp;
    }
    
    
    
    /**
     * Getter method for tls property
     * @param obj IN connection object
     * @return tls
     */
    static inline flom_tls_t *flom_conn_get_tls(flom_conn_t *obj) {
        return obj->tls;
    }
    
    
    
    /**
     * Getter method for msg property
     * @param obj IN connection object
     * @return msg
     */
    static inline struct flom_msg_s *flom_conn_get_msg(flom_conn_t *obj) {
        return obj->msg;
    }
    
    
    
    /**
     * Setter method for msg property
     * @param obj IN/OUT connection object
     * @param value IN new value for msg
     */
    static inline void flom_conn_set_msg(flom_conn_t *obj,
                                         struct flom_msg_s *value) {
        obj->msg = value;
    }


    
    /**
     * Getter method for parser property
     * @param obj IN connection object
     * @return msg
     */
    static inline GMarkupParseContext *flom_conn_get_parser(flom_conn_t *obj) {
        return obj->parser;
    }
    
    
    
    /**
     * Setter method for parser property
     * @param obj IN connection object
     * @param ref IN reference to the new parser
     * @return msg
     */
    static inline void flom_conn_set_parser(
        flom_conn_t *obj, GMarkupParseContext *ref) {
        if (NULL != obj->parser)
            g_markup_parse_context_free(obj->parser);
        obj->parser = ref;
    }
    
    

    /**
     * Release the parser object used by the connection
     * @param obj IN/OUT connection object
     */
    void flom_conn_free_parser(flom_conn_t *obj);



    /**
     * Receive a buffer using raw TCP/IP or TLS over TCP/IP
     * @param obj IN/OUT TLS object
     * @param buf OUT buffer to send
     * @param len IN buffer lenght
     * @param received OUT number of read bytes
     * @param timeout IN maximum wait time to receive the answer
     *        (milliseconds); use @ref FLOM_NETWORK_WAIT_TIMEOUT for indefinite
     *        wait
     * @param src_addr OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page); it applies only to
     *                 UDP/IP
     * @param addrlen OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page); it applied only to
     *                 UDP/IP
     * @return a reason code
     */
    int flom_conn_recv(flom_conn_t *obj, void *buf, size_t len,
                       size_t *received, int timeout,
                       struct sockaddr *src_addr, socklen_t *addrlen);


    
    /**
     * Send a buffer using raw TCP/IP or TLS over TCP/IP
     * @param obj IN/OUT connection object
     * @param buf IN buffer to send
     * @param len IN buffer lenght
     * @return a reason code
     */
    int flom_conn_send(flom_conn_t *obj, const void *buf, size_t len);



    /**
     * Close a raw TCP/IP or TLS over TCP/IP connection
     * @param obj IN/OUT connection object
     * @return a reason code
     */
    static inline int flom_conn_close(flom_conn_t *obj) {
        return flom_tcp_close(&obj->tcp);
    }
    


    /**
     * Start connection termination
     * @param obj IN/OUT connection object
     * @return a reason code
     */
    int flom_conn_terminate(flom_conn_t *obj);

    
    
    /**
     * Initialize the TLS object related to a connection
     * @param obj IN/OUT connection object
     * @param client IN boolean value: TRUE for TLS client connection, FALSE
     *                  for TLS server connection
     */
    static inline int flom_conn_init_tls(flom_conn_t *obj, int client) {
        if (NULL == (obj->tls = flom_tls_new(client)))
            return FLOM_RC_NEW_OBJ;
        return FLOM_RC_OK;
    }


    
    /**
     * Trace the content of a connection data struct
     * @param conn IN connection object to trace
     */
    void flom_conn_trace(const flom_conn_t *conn);



    /**
     * Perform connection authentication
     * @param conn IN/OUT connection object
     * @param peer_id IN unique ID passed by the peer during handshake
     * @param tls_check_peer_id IN boolean value
     * @return a reason code
     */
    int flom_conn_authenticate(flom_conn_t *conn, const gchar *peer_id,
                               int tls_check_peer_id);


    
    /**
     * Set SO_KEEPALIVE and correlated parameters for the socket associated
     * to the passed file descriptor.
     * This function has no relation with flom_conn_t objects because it's
     * used only on accepted socket; it could be refactored when
     * flom_conns_add will be add a flom_conn_t object...
     * @param config IN configuration object, NULL for global config
     * @param fd IN/OUT socket file descriptor
     * @return a reason code
     */
    int flom_conn_set_keepalive(flom_config_t *config, int fd);



    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_CONN_H */
