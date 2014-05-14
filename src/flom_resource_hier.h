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
#ifndef FLOM_RESOURCE_HIER_H
# define FLOM_RESOURCE_HIER_H



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
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_RESOURCE_HIER



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    /**
     * Check if a lock can be granted on a resource
     * @param resource IN reference to resource object
     * @param lock IN lock mode to check
     * @return a boolean value
     */
    int flom_resource_hier_can_lock(flom_resource_t *resource,
                                      flom_lock_mode_t lock);


    
    /**
     * Initialize a new resource of type hierarchical
     * @param resource IN reference to resource object
     * @param name IN resource name as asked by the client
     * @return a reason code
     */
    int flom_resource_hier_init(flom_resource_t *resource,
                                  const gchar *name);

    

    /**
     * Manage an incoming message for a "hierarchical" resource
     * @param resource IN/OUT reference to resource object
     * @param conn IN connection reference
     * @param msg IN reference to incoming message
     * @return a reason code
     */
    int flom_resource_hier_inmsg(flom_resource_t *resource,
                                   struct flom_conn_data_s *conn,
                                   struct flom_msg_s *msg);


    
    /**
     * Manage an clean-up signal for a "hierarchical" resource
     * @param resource IN/OUT reference to resource object
     * @param conn IN connection reference
     * @return a reason code
     */
    int flom_resource_hier_clean(flom_resource_t *resource,
                                   struct flom_conn_data_s *conn);



    /**
     * Destroy a hierarchical resource (frees holders list and waitings queue)
     * @param resource IN/OUT reference to resource object
     */
    void flom_resource_hier_free(flom_resource_t *resource);


    
    /**
     * Recursively frees all leaves elements and then this one
     * @param element IN/OUT tree element to free
     */
    void flom_resource_hier_free_element(
        struct flom_rsrc_data_hier_element_s *element);



    /**
     * Compare the name of the current resource and an external name passed
     * to the method
     * @param resource IN reference to this resource object
     * @param name IN another resource name to compare with this resource name
     * @return -1,0,+1 as strcmp
     */
    int flom_resource_hier_compare_name(const flom_resource_t *resource,
                                        const gchar *name);



    /**
     * Check if any of the lock waitings can get a lock
     * @param resource IN/OUT reference to resource object
     * @return a reason code
     */
    int flom_resource_hier_waitings(flom_resource_t *resource);



#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_RESOURCE_HIER_H */
