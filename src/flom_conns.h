/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_CONNS_H
# define FLOM_CONNS_H



#include <config.h>



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



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_CONNS



/**
 * Default allocation size for @ref flom_conns_t objects
 */
#define FLOM_CONNS_DEFAULT_ALLOCATION  10
/**
 * Expansion allocation step
 */
#define FLOM_CONNS_STEP_ALLOCATION     1.2
/**
 * Null file descriptor
 */
#define NULL_FD   -1



/**
 * A structured object used to store addresses
 */
struct flom_addr_s {
    /**
     * Client address len
     */
    socklen_t addr_len;
    union {
        /**
         * Client addressed for generic connections
         */
        struct sockaddr    sa;
        /**
         * Client addresses for AF_UNIX connections
         */
        struct sockaddr_un saun;
        /**
         * Client addresses for AF_INET connections
         */
        struct sockaddr_in sain;
    };
};



/**
 * A structured object used to register connections
 */
struct flom_conns_s {
    /**
     * Number of allocated connections
     */
    nfds_t allocated;
    /**
     * Number of used connections
     */
    nfds_t used;
    /**
     * Array used for poll function
     */
    struct pollfd *fds;
    /**
     * Connection domain as specified when calling socket function
     */
    int domain;
    /**
     * Array of addresses
     */
    struct flom_addr_s *addr;
};
    


typedef struct flom_conns_s flom_conns_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize a new object
     * @param conns IN/OUT object to be initialized
     * @param domain IN socket domain for all the connections managed by this
     *                  object
     * @return a reason code
     */
    int flom_conns_init(flom_conns_t *conns, int domain);



    /**
     * Add a new connection
     * @param conns IN/OUT connections object
     * @param fd IN file descriptor
     * @param domain IN socket domain
     * @param addr_len IN lenght of address
     * @param sa IN address
     * @return a reason code
     */
    int flom_conns_add(flom_conns_t *conns, int fd,
                       socklen_t addr_len, const struct sockaddr *sa);



    /**
     * Return the number of active connections managed by the object
     * @param conns IN connections object
     * @return the number of active connections
     */
    static inline nfds_t flom_conns_get_used(const flom_conns_t *conns) {
        return conns->used;
    }

    

    /**
     * Return a file descriptor associated to a connection
     * @param conns IN connections object
     * @param id IN identificator (position in array) of the connection
     * @return the associated file descriptor or @ref NULL_FD if any error
     *         happens
     */
    static inline int flom_conns_get_fd(const flom_conns_t *conns, int id) {
        if (id < conns->used)
            return conns->fds[id].fd;
        else
            return NULL_FD;
    }



    /**
     * @return the fds array to be used with poll function
     */
    static inline struct pollfd *flom_conns_get_fds(flom_conns_t *conns) {
        return conns->fds;
    }


    
    /**
     * Set events field for every connection in the object
     * @param conns IN/OUT connections object
     * @param events IN new value for every events field
     * @return a reason code
     */
    int flom_conns_set_events(flom_conns_t *conns, short events);
    


    /**
     * Expand object size
     * @param conns IN/OUT connections object
     * @return a reason code
     */
    int flom_conns_expand(flom_conns_t *conns);

    
    
    /**
     * Close a file descriptor and set it to @ref NULL_FD; use
     * @ref flom_conns_clean to remove the connections associated to closed
     * file descriptors
     * @param conns IN/OUT connections object
     * @param id IN connection must be closed
     * @return a reason code
     */
    int flom_conns_close_fd(flom_conns_t *conns, nfds_t id);

    

    /**
     * Remove connections with invalid (closed) file descriptor
     * @param conns IN/OUT connections object
     * @return a reason code
     */
    int flom_conns_clean(flom_conns_t *conns);

    
    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_CONNS_H */
