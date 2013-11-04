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
#ifndef FLOM_LOCKER_H
# define FLOM_LOCKER_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_conns.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_LOCKER



/**
 * Data structure used for a locker thread
 */
struct flom_locker_s {
    /**
     * Identifier of the thread running the locker
     */
    GThread  *thread;
    /**
     * Pipe file descriptor: used by main thread (listener) to send commands
     */
    int       write_pipe;
    /**
     * Pipe file descriptor: used by locker thread to receive commands
     */
    int       read_pipe;
    /**
     * Resource managed by this locker; this is the key to pick-up the right
     * locker from a pool
     */
    gchar    *resource_name;
};



/**
 * A pool of lockers
 */
struct flom_locker_array_s {
    /**
     * Number of available lockers
     */
    gint       n;
    /**
     * Array of lockers
     */
    GPtrArray *array;
};



/**
 * A shorthand to avoid "struct" verb
 */
typedef struct flom_locker_array_s flom_locker_array_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize a locker struct
     * @param locker IN/OUT struct to be initialized
     */
    static inline void flom_locker_init(struct flom_locker_s *locker) {
        locker->thread = NULL;
        locker->write_pipe = locker->read_pipe = NULL_FD;
        locker->resource_name = NULL;
    }

    

    /**
     * Destroy the objects connected to the locker and the locker object
     * itself (the pointer is not anymore valid after this call)
     * @param locker IN/OUT object to destroy (remove from memory)
     */
    void     flom_locker_destroy(struct flom_locker_s *locker);

    

    /**
     * Initialize an array of lockers
     * @param lockers IN/OUT pointer to object to initialize
     */
    void     flom_locker_array_init(flom_locker_array_t *lockers);

    

    /**
     * Add a new locker to locker array
     * @param lockers IN/OUT array of lockers
     * @param locker IN new locker to add
     */
    void     flom_locker_array_add(flom_locker_array_t *lockers,
                                   struct flom_locker_s *locker);



    /**
     * Main loop function for locker thread
     * @param data IN pointer to locker context, it must be a pointer to
     * @ref flom_locker_s
     */
    gpointer flom_locker_loop(gpointer data);


    
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
