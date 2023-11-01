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
     * @param node IN reference to root node of a resource
     * @param lock IN lock mode to check
     * @param level_name IN a pointer to the current level part of the name
     * @return a boolean value
     */
    int flom_resource_hier_can_lock(struct flom_rsrc_data_hier_element_s *node,
                                    flom_lock_mode_t lock, gchar **level_name);



    /**
     * Add a new locker to a resource
     * @param resource IN/OUT resource reference
     * @param cl IN connection lock of the locker
     * @param splitted_name IN name of the asked resource to must be locked
     */
    int flom_resource_hier_add_locker(flom_resource_t *resource,
                                      struct flom_rsrc_conn_lock_s *cl,
                                      gchar **splitted_name);



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
     * @param locker_uid IN unique identifier or the locker that's managing
     *        the resource
     * @param conn IN connection reference
     * @param msg IN reference to incoming message
     * @param next_deadline OUT next deadline asked by the resource (the
     *        resource is waiting a time-out)
     * @return a reason code
     */
    int flom_resource_hier_inmsg(flom_resource_t *resource,
                                 flom_uid_t locker_uid,
                                 flom_conn_t *conn,
                                 struct flom_msg_s *msg,
                                 struct timeval *next_deadline);


    
    /**
     * Manage an clean-up signal for a "hierarchical" resource
     * @param resource IN/OUT reference to resource object
     * @param conn IN connection reference
     * @return a reason code
     */
    int flom_resource_hier_clean(flom_resource_t *resource,
                                 flom_conn_t *conn);



    /**
     * Garbage collector: it removes useless leaves from the tree
     * @param element IN/OUT tree element to free
     */
    void flom_resource_hier_gc(
        struct flom_rsrc_data_hier_element_s *element);



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
     * Change current resource name to a new one; for hierarchical resources
     * global name is one of the many names the resource can assume (it's a
     * tree of names!). The name property is used as transitory
     * @param resource IN referente to resource object
     * @param name IN new name to be assigned
     * @return a reason code
     */
    int flom_resource_hier_change_name(flom_resource_t *resource,
                                       const gchar *name);

    

    /**
     * Check if any of the lock waitings can get a lock
     * @param resource IN/OUT reference to resource object
     * @return a reason code
     */
    int flom_resource_hier_waitings(flom_resource_t *resource);



    /**
     * Trace the content of a hierarchical resource
     * @param resource IN reference to resource object
     */
    void flom_resource_hier_trace(const flom_resource_t *resource);



    /**
     * Trace the content of a hierarchical resource (recursive helper function
     * for flom_resource_hier_trace)
     * @param node IN reference to node object
     * @param level IN level of node to trace
     */
    void flom_resource_hier_trace_node(
        struct flom_rsrc_data_hier_element_s *node, int level);


    
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
