/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_LOCKER_H
# define FLOM_LOCKER_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_conns.h"
#include "flom_rsrc.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_LOCKER



/**
 * Used to break poll loop when looking for new/old clients
 * This constant will probably become useless after "ping" implementation
 */
#define FLOM_LOCKER_POLL_TIMEOUT   1000



/**
 * Data structure used for a locker thread
 */
struct flom_locker_s {
    /**
     * Identifier of the thread running the locker
     */
    GThread                 *thread;
    /**
     * Unique identifier associated to the locker object
     */
    flom_uid_t               uid;
    /**
     * Pipe file descriptor: used by main thread (listener) to send commands
     */
    int                      write_pipe;
    /**
     * Pipe file descriptor: used by locker thread to receive commands
     */
    int                      read_pipe;
    /**
     * Last sequence number sent by parent (listener) to locker thread:
     * parent point of view
     */
    int                      write_sequence;
    /**
     * Last sequence read by locker thread and sent by parent (listener):
     * child point of view
     */
    int                      read_sequence;
    /**
     * Number of polling periods the locker thread performed nothing (without
     * any client)
     */
    int                      idle_periods;
    /**
     * Minimum number of milliseconds a resource (and a locker) must be kept
     * after last usage
     */
    int                      idle_lifespan;
    /**
     * Resource managed by the locker
     */
    flom_resource_t          resource;
};



/**
 * A pool of lockers
 */
struct flom_locker_array_s {
    /*
      @@@ add a mutex to serialize the object and allows VFS to read
      consistent data
    */
    /**
     * Array of lockers
     */
    GPtrArray *array;
};



/**
 * A shorthand to avoid "struct" verb
 */
typedef struct flom_locker_array_s flom_locker_array_t;



/**
 * It's the struct passed from parent thread (listener) to child thread
 * (locker) when a new client arrive
 */
struct flom_locker_token_s {
    /**
     * Socket domain associated to client connection
     */
    int domain;
    /**
     * Client connection (accepted) file descriptor
     */
    int client_fd;
    /**
     * Sequence number associated to the token
     */
    int sequence;
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize a locker struct
     * @param locker IN/OUT struct to be initialized
     * @return a reason code
     */
    static inline void flom_locker_init(struct flom_locker_s *locker) {
        locker->thread = NULL;
        locker->uid = 0;
        locker->write_pipe = locker->read_pipe = FLOM_NULL_FD;
        locker->write_sequence = locker->read_sequence =
            locker->idle_periods = 0;
        memset(&locker->resource, 0, sizeof(flom_resource_t));
    }

    

    /**
     * Destroy the objects connected to the locker and the locker object
     * itself (the pointer is not anymore valid after this call)
     * @param locker IN/OUT object to destroy (remove from memory)
     */
    void flom_locker_destroy(struct flom_locker_s *locker);



    /**
     * Initialize an array of lockers
     * @param lockers IN/OUT pointer to object to initialize
     */
    void flom_locker_array_init(flom_locker_array_t *lockers);

    

    /**
     * Remove all objects pointed by locker array and the array itself
     * @param lockers IN/OUT pointer to object to release
     */
    void flom_locker_array_free(flom_locker_array_t *lockers);


    
    /**
     * Add a new locker to locker array
     * @param lockers IN/OUT array of lockers
     * @param locker IN new locker to add
     */
    void flom_locker_array_add(flom_locker_array_t *lockers,
                               struct flom_locker_s *locker);



    /**
     * Remove a locker from locker array
     * @param lockers IN/OUT array of lockers
     * @param locker IN pointer to the element must be deleted
     */
    void flom_locker_array_del(flom_locker_array_t *lockers,
                               struct flom_locker_s *locker);

    
                                   
    /**
     * Number of active lockers thread
     * @param lockers IN/OUT array of lockers
     * @return how many lockers are managed by the object
     */
    static inline guint flom_locker_array_count(
        const flom_locker_array_t *lockers) {
        return lockers->array->len;
    }



    /**
     * Retrieve a pointer to a locker
     * @param lockers IN/OUT array of lockers
     * @param i IN index of the desired element
     * @return NULL if i is an invalid index, the desired locker otherwise
     */
    static inline struct flom_locker_s *flom_locker_array_get(
        flom_locker_array_t *lockers, guint i) {
        if (i < 0 || i >= lockers->array->len)
            return NULL;
        else
            return g_ptr_array_index(lockers->array, i);
    }


    
    /**
     * Main loop function for locker thread
     * @param data IN pointer to locker context, it must be a pointer to
     * @ref flom_locker_s
     */
    gpointer flom_locker_loop(gpointer data);


    
    /**
     * Manager POLLIN event received from locker thread
     * @param locker IN/OUT locker context object
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @param refresh_conns OUT the conns object must be refreshed due to
     *        some deletion inside it
     * @param next_deadline OUT next deadline asked by the resource (the
     *        resource is waiting a time-out)
     * @return a reason code
     */
    int flom_locker_loop_pollin(struct flom_locker_s *locker,
                                flom_conns_t *conns, guint id,
                                int *refresh_conns,
                                struct timeval *next_deadline);



    /**
     * Compute the timeout for the poll loop from the current time and the
     * deadline requested by the resource
     * @param next_deadline IN point in time for the timeout requested by the
     *        resource
     * @return a timeout value or -1 if next_deadline is in the past
     */
    int flom_locker_loop_get_timeout(const struct timeval *next_deadline);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_LOCKER_H */
