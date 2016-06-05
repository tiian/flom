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
#ifndef FLOM_RESOURCE_TIMESTAMP_H
# define FLOM_RESOURCE_TIMESTAMP_H



#include <config.h>



#include "flom_msg.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_RESOURCE_TIMESTAMP



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    /**
     * Check if a lock can be granted on a resource
     * @param resource IN reference to resource object
     * @return a boolean value
     */
    int flom_resource_timestamp_can_lock(flom_resource_t *resource);



    /**
     * Compute the next deadline starting from last timestamp provided and the
     * minimum timeval between two timestamps
     * @param resource IN reference to resource object
     * @return next deadline
     */
    struct timeval flom_resource_timestamp_next_deadline(
        flom_resource_t *resource);

 

    /**
     * Compute the next timestamp value
     * @param resource IN/OUT reference to resource object
     * @param timestamp OUT the timeval struct where the timestamp will be
     *        saved 
     * @param str_timestamp OUT the buffer necessary to store the timestamp
     * @param max IN size of the buffer as required by function strftime
     * @return a reason code
     */
    int flom_resource_timestamp_get(flom_resource_t *resource,
                                    struct timeval *timestamp,
                                    gchar *str_timestamp, size_t max);


    
    /**
     * Initialize a new resource of type timestamp
     * @param resource IN reference to resource object
     * @param name IN resource name as asked by the client
     * @return a reason code
     */
    int flom_resource_timestamp_init(flom_resource_t *resource,
                                     const gchar *name);

    

    /**
     * Manage an incoming message for a "timestamp" resource
     * @param resource IN/OUT reference to resource object
     * @param conn IN connection reference
     * @param msg IN reference to incoming message
     * @param next_deadline OUT next deadline asked by the resource (the
     *        resource is waiting a time-out)
     * @return a reason code
     */
    int flom_resource_timestamp_inmsg(flom_resource_t *resource,
                                      flom_conn_t *conn,
                                      struct flom_msg_s *msg,
                                      struct timeval *next_deadline);


    
    /**
     * Manage an clean-up signal for a "timestamp" resource
     * @param resource IN/OUT reference to resource object
     * @param conn IN connection reference
     * @return a reason code
     */
    int flom_resource_timestamp_clean(flom_resource_t *resource,
                                      flom_conn_t *conn);



    /**
     * Destroy a timestamp resource (frees holders list and waitings queue)
     * @param resource IN/OUT reference to resource object
     */
    void flom_resource_timestamp_free(flom_resource_t *resource);



    /**
     * Timeout expiration: a new timestamp can be generated
     * @param resource IN/OUT reference to resource object
     * @param next_deadline OUT next deadline asked by the resource (the
     *        resource is waiting a time-out)
     * @return a reason code
     */
    int flom_resource_timestamp_timeout(flom_resource_t *resource,
                                        struct timeval *next_deadline);

    
    
    /**
     * Check if any of the lock waitings can get a lock
     * @param resource IN/OUT reference to resource object
     * @return a reason code
     */
    int flom_resource_timestamp_waitings(flom_resource_t *resource);



#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_RESOURCE_TIMESTAMP_H */
