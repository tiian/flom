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
     * Size of the address (see @ref address field)
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
     * Getter method for sockfd property
     * @param obj IN TCP connection object
     * @return socket file descriptor
     */
    static inline int flom_tcp_get_sockfd(const flom_tcp_t *obj) {
        return obj->sockfd; }


    
    /**
     * Setter method for sockfd property
     * @param obj IN TCP connection object
     * @param sockfd IN socket file descriptor
     * @return the same value passed with sockfd parameter
     */
    static inline int flom_tcp_set_sockfd(flom_tcp_t *obj, int sockfd) {
        obj->sockfd = sockfd;
        return sockfd; }



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
     * @param socket_type IN socket type
     */
    static inline void flom_tcp_set_socket_type(
        flom_tcp_t *obj, int socket_type) {
        obj->socket_type = socket_type; }



    /**
     * Getter method for addrlen property
     * @param obj IN TCP connection object
     * @return the lenght of the TCP address
     */
    static inline size_t flom_tcp_get_addrlen(const flom_tcp_t *obj) {
        return obj->addrlen; }


    
    /**
     * Getter method for address property
     * @param obj IN TCP connection object
     * @return the TCP address
     */
    static inline struct sockaddr *flom_tcp_get_address(flom_tcp_t *obj) {
        return &obj->sa; }
    
    
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
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param type IN file descriptor associated type (SOCK_STREAM or
     *             SOCK_DGRAM)
     * @param buf OUT buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @param read_bytes OUT number of bytes read, XML message length
     * @param timeout IN maximum wait time to receive the answer (milliseconds)
     * @param src_addr OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @param addrlen OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @return a reason code
     */
    int flom_tcp_retrieve(int fd, int type, char *buf, size_t buf_size,
                          ssize_t *read_bytes, int timeout,
                          struct sockaddr *src_addr, socklen_t *addrlen);



    /**
     * Send a message to a TCP/IP socket (file descriptor)
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param buf IN buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @return a reason code
     */
    int flom_tcp_send(int fd, const char *buf, size_t buf_size);

    
    
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



