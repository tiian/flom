/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
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



#define FLOM_CONNS_DEFAULT_ALLOCATION  10


/**
 * A structured object used to store addresses
 */
struct flom_addr_s {
    /**
     * Client address len
     */
    socklen_t clilen;
    union {
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
    int allocated;
    /**
     * Number of used connections
     */
    int used;
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
     * @param fc IN/OUT object to be initialized
     * @reason a reason code
     */
    int flom_conns_init(flom_conns_t *fc);


    
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
