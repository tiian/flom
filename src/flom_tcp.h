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
#ifndef FLOM_TCP_H
# define FLOM_TCP_H



#include <config.h>



#include "flom_config.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_TCP



/**
 * Struct used to manipulate a TCP connection
 */
typedef struct {
    /**
     * Configuration reference or NULL (default configuration)
     */
    flom_config_t            *config;
    /**
     * Communication domain/family (AF_INET, AF_INET6)
     */
    int                       domain;
    /**
     * Socket file descriptor
     */
    int                       sockfd;
    /**
     * Socket type: SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
     */
    int                       socket_type;
    /**
     * Size of the address field
     */
    size_t                    addrlen;
    /**
     * Peer address
     */
    union {
        /**
         * Client address for generic connections
         */
        struct sockaddr             sa;
        /**
         * Client address for AF_UNIX connections
         */
        struct sockaddr_un          sa_un;
        /**
         * Client address for AF_INET connections
         */
        struct sockaddr_in          sa_in;
        /**
         * Client address for AF_INET6 connections
         */
        struct sockaddr_in6         sa_in6;
        /**
         * Maximum size necessary to store an address
         */
        struct sockaddr_storage     sa_storage;
    };
} flom_tcp_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize a flom_tcp_t object
     * @param obj IN/OUT to be initialized
     * @param config IN a reference to configuration object or NULL
     */
    void flom_tcp_init(flom_tcp_t *obj, flom_config_t *config);
    


    /**
     * Getter method for domain property
     * @param obj IN TCP connection object
     * @return socket file descriptor
     */
    static inline int flom_tcp_get_domain(const flom_tcp_t *obj) {
        return obj->domain; }



    /**
     * Setter method for domain property
     * @param obj IN TCP connection object
     * @param value IN new value for domain
     * @return socket file descriptor
     */
    static inline void flom_tcp_set_domain(flom_tcp_t *obj, int value) {
        obj->domain = value; }



    /**
     * Getter method for sockfd property
     * @param obj IN TCP connection object
     * @return socket file descriptor
     */
    static inline int flom_tcp_get_sockfd(const flom_tcp_t *obj) {
        return obj->sockfd; }


    
    /**
     * Setter method for sockfd property
     * @param obj IN TCP connection object
     * @param value IN new value for socket file descriptor
     * @return the same value passed with sockfd parameter
     */
    static inline int flom_tcp_set_sockfd(flom_tcp_t *obj, int value) {
        obj->sockfd = value;
        return obj->sockfd; }



    /**
     * Getter method for socket_type property
     * @param obj IN TCP connection object
     * @return socket file descriptor
     */
    static inline int flom_tcp_get_socket_type(const flom_tcp_t *obj) {
        return obj->socket_type; }


    
    /**
     * Setter method for socket_type property
     * @param obj IN TCP connection object
     * @param value IN new value for socket type
     */
    static inline void flom_tcp_set_socket_type(
        flom_tcp_t *obj, int value) {
        obj->socket_type = value; }



    /**
     * Getter method for addrlen property
     * @param obj IN TCP connection object
     * @return the lenght of the TCP address
     */
    static inline size_t flom_tcp_get_addrlen(const flom_tcp_t *obj) {
        return obj->addrlen; }


    
    /**
     * Setter method for addrlen property
     * @param obj IN/OUT TCP connection object
     * @param value IN new value for addrlen property
     * @return the lenght of the TCP address
     */
    static inline void flom_tcp_set_addrlen(flom_tcp_t *obj, size_t value) {
        obj->addrlen = value; }


    
    /**
     * Getter method for storage address
     * @param obj IN TCP connection object
     * @return the TCP address
     */
    static inline struct sockaddr_storage *flom_tcp_get_sa_storage(
        flom_tcp_t *obj) {
        return &obj->sa_storage; }
    


    /**
     * Setter method for storage address: it can be used to store any valid
     * type of address that can be managed by struct sockaddr_storage
     * @param obj IN/OUT TCP connection object
     * @param sa IN socket address
     * @param sa_len IN socket address lenght
     */
    static inline void flom_tcp_set_sa_storage(
        flom_tcp_t *obj, const struct sockaddr_storage *sa, size_t sa_len) {
        memcpy(&obj->sa_storage, sa, sa_len);
        obj->addrlen = sa_len;
    }

    

    /**
     * Getter method for generic address
     * @param obj IN TCP connection object
     * @return the generic address
     */
    static inline struct sockaddr *flom_tcp_get_sa(flom_tcp_t *obj) {
        return &obj->sa; }
    


    /**
     * Getter method for UNIX address
     * @param obj IN TCP connection object
     * @return the UNIX address
     */
    static inline struct sockaddr_un *flom_tcp_get_sa_un(flom_tcp_t *obj) {
        return &obj->sa_un; }


    
    
    /**
     * Getter method for IPv4 address
     * @param obj IN TCP connection object
     * @return the IPv4 address
     */
    static inline struct sockaddr_in *flom_tcp_get_sa_in(flom_tcp_t *obj) {
        return &obj->sa_in; }


    
    
    /**
     * Getter method for IPv6 address
     * @param obj IN TCP connection object
     * @return the IPv6 address
     */
    static inline struct sockaddr_in6 *flom_tcp_get_sa_in6(flom_tcp_t *obj) {
        return &obj->sa_in6; }


    
    
    /**
     * Prepare an incoming TCP/IP connection ready to accept new clients
     * @param obj IN/OUT TCP communication object
     * @return a reason code
     */
    int flom_tcp_listen(flom_tcp_t *obj);



    /**
     * Try to establish a TCP/IP connection; it must considered a "private"
     * method because it's used only by @ref flom_tcp_connect
     * @param config IN configuration object, NULL for global config
     * @param gai IN result obtained by getaddrinfo function
     * @param fd OUT file descriptor associated to the connected socket
     * @return the pointer to the element successfully connected, NULL if no
     *         element is available
     */
    const struct addrinfo *flom_tcp_try_connect(
        flom_config_t *config, const struct addrinfo *gai, int *fd);
    


    /**
     * Establish a TCP/IP connection peeking address, port and interface from
     * configuration
     * @param obj IN/OUT TCP communication object
     * @return a reason code
     */
    int flom_tcp_connect(flom_tcp_t *obj);


    
    /**
     * Retrieve the first XML message from a TCP/IP socket (file descriptor)
     * @param obj IN TCP communication object
     * @param buf OUT buffer will be used to store the XML message
     * @param len IN size of buf
     * @param received OUT number of bytes read, XML message length
     * @param src_addr OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @param addrlen OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @return a reason code
     */
    int flom_tcp_recv(const flom_tcp_t *obj, void *buf, size_t len,
                      size_t *received,
                      struct sockaddr *src_addr, socklen_t *addrlen);



    /**
     * Send a message to a TCP/IP socket (file descriptor)
     * @param obj IN TCP communication object
     * @param buf IN buffer will be used to store the XML message
     * @param len IN size of buf
     * @return a reason code
     */
    int flom_tcp_send(const flom_tcp_t *obj, const void *buf, size_t len);

    

    /**
     * Close a TCP/IP connection
     * @param obj IN/OUT TCP communication object
     * @return a reason code
     */
    int flom_tcp_close(flom_tcp_t *obj);



    /**
     * Retrieve a string containing IP address and port of the connected
     * peer
     * @param obj IN TCP communication object
     * @return a string that must be released by the caller using g_free
     */
    gchar *flom_tcp_retrieve_peer_name(const flom_tcp_t *obj);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_TCP_H */



