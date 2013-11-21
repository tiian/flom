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



#include "flom_msg.h"



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
#define FLOM_CONNS_PERCENT_ALLOCATION  20
/**
 * Null file descriptor
 */
#define NULL_FD   -1
/**
 * Transferred file descriptor
 */
#define TRNS_FD   -2



/**
 * A structured object used to store connection data
 */
struct flom_conn_data_s {
    /**
     * File descriptor associated to the connection
     */
    int       fd;
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
    /**
     * Last received message (allocated by malloc)
     */
    struct flom_msg_s     *msg;
    /**
     * GMarkup Parser context (allocated by g_markup_parse_context_new)
     */
    GMarkupParseContext   *gmpc;
};



/**
 * A structured object used to register connections
 */
struct flom_conns_s {
    /* @@@ remove me
     * Number of allocated connections
    nfds_t allocated;
     */
    /* @@@ remove me
     * Number of used connections
    nfds_t used;
     */
    /**
     * Array used for poll function (it must be re-generated before poll
     * function)
     */
    struct pollfd *poll_array;
    /* @@@ remove me!
    struct pollfd *fds;
    */
    /**
     * Connection domain as specified when calling socket function
     */
    int domain;
    /**
     * Array of connection data (it's an array of pointer for @ref
     * flom_conn_data_s)
     */
    GPtrArray     *array;
    /**
     * Array size
     */
    guint          n;
    /* @@@ remove me! 
    struct flom_conn_data_s *cd;
    */
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
     */
    void flom_conns_init(flom_conns_t *conns, int domain);



    /**
     * Add a new connection
     * @param conns IN/OUT connections object
     * @param fd IN file descriptor
     * @param addr_len IN lenght of address
     * @param sa IN address
     * @return a reason code
     */
    int flom_conns_add(flom_conns_t *conns, int fd,
                       socklen_t addr_len, const struct sockaddr *sa);



    
    /**
     * Import a connection: the imported connection (@ref flom_conn_data_s)
     * must not be freed by the caller because the import does not make a copy
     * of the structure, it picks up the passed reference
     * @param conns IN/OUT connections object
     * @param fd IN file descriptor
     * @param cd IN connection data struct
     */
    void flom_conns_import(flom_conns_t *conns, int fd,
                           struct flom_conn_data_s *cd);

    

    /**
     * Return the socket domain associated with all the connection
     * @return socket domain
     */
    static inline int flom_conns_get_domain(const flom_conns_t *conns) {
        return conns->domain;
    }



    /**
     * Set the socket domain of an already initialized connections object;
     * pay attention this may invalidate the already stored addresses and
     * should be used only when domain is not available at initialization
     * time
     * @param conns IN/OUT connections object
     * @param domain IN the new domain of the connections object
     */
    static inline void flom_conns_set_domain(flom_conns_t *conns,
                                             int domain) {
        conns->domain = domain;
    }

    

    /**
     * Return the number of active connections managed by the object
     * @param conns IN connections object
     * @return the number of active connections
     */
    static inline guint flom_conns_get_used(const flom_conns_t *conns) {
        return conns->n;
    }

    

    /**
     * Return the file descriptor associated to a connection
     * @param conns IN connections object
     * @param id IN identificator (position in array) of the connection
     * @return the associated file descriptor or @ref NULL_FD if any error
     *         happens
     */
    static inline int flom_conns_get_fd(const flom_conns_t *conns, guint id) {
        if (id < conns->n)
            return ((struct flom_conn_data_s *)
                    g_ptr_array_index(conns->array, id))->fd;
        else
            return NULL_FD;
    }



    /**
     * Return a reference (read-only pointer) to the struct containing
     * connection data of a specific connection
     * @param conns IN connections object
     * @param id IN identificator (position in array) of the connection
     * @return a reference to the asked structure or NULL
     */
    static inline struct flom_conn_data_s *flom_conns_get_cd(
        const flom_conns_t *conns, guint id) {
        if (id < conns->n)
            return ((struct flom_conn_data_s *)
                    g_ptr_array_index(conns->array, id));
        else
            return NULL;
    }



    /**
     * Build a new array for poll function and return it
     * @param conns IN connections object
     * @return the fds array to be used with poll function or NULL if an
     *         error happens
     */
    struct pollfd *flom_conns_get_fds(flom_conns_t *conns);



    /**
     * Return the message associated to a connection
     * @param conns IN connections object
     * @param id IN identificator (position in array) of the connection
     * @return the associated message or NULL if any error happens
     */
    static inline struct flom_msg_s *flom_conns_get_msg(
        flom_conns_t *conns, guint id) {
        if (id < conns->n)
            return ((struct flom_conn_data_s *)
                    g_ptr_array_index(conns->array, id))->msg;
        else
            return NULL;
    }



    /**
     * Return the GMarkupParseContext associated to a connection
     * @param conns IN connections object
     * @param id IN identificator (position in array) of the connection
     * @return the associated GMarkupParseContext or NULL if any error happens
     */
    static inline GMarkupParseContext *flom_conns_get_gmpc(        
        flom_conns_t *conns, guint id) {
        if (id < conns->n)
            return ((struct flom_conn_data_s *)
                    g_ptr_array_index(conns->array, id))->gmpc;
        else
            return NULL;
    }



    /**
     * Set events field for every connection in the object
     * NOTE: it must be called after @ref flom_conns_get_fds because it
     *       resize the poll array if necessary
     * @param conns IN/OUT connections object
     * @param events IN new value for every events field
     * @return a reason code
     */
    int flom_conns_set_events(flom_conns_t *conns, short events);
    


    /**
     * Close a file descriptor and set it to @ref NULL_FD; use
     * @ref flom_conns_clean to remove the connections associated to closed
     * file descriptors
     * @param conns IN/OUT connections object
     * @param id IN connection must be closed
     * @return a reason code
     */
    int flom_conns_close_fd(flom_conns_t *conns, guint id);



    /**
     * Mark as "transferred to another thread" the file descriptor associated
     * to a connection
     * @param conns IN/OUT connections object
     * @param id IN connection must be marked
     * @return a reason code
     */
    int flom_conns_trns_fd(flom_conns_t *conns, guint id);

    

    /**
     * Remove connections with invalid (closed) file descriptor
     * @param conns IN/OUT connections object
     */
    void flom_conns_clean(flom_conns_t *conns);

    

    /**
     * Free all the memory allocated by connections object
     * @param conns IN/OUT connections object
     */
    void flom_conns_free(flom_conns_t *conns);



    /**
     * Trace the content of a connection data struct
     * @param conn IN connection data to trace
     */
    void flom_conn_data_trace(const struct flom_conn_data_s *conn);


    
    /**
     * Trace the content of a connections object
     * @param conns IN connections object to trace
     */
    void flom_conns_trace(const flom_conns_t *conns);


    
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
