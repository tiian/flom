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
#ifndef FLOM_RSRC_H
# define FLOM_RSRC_H



#include <config.h>



#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif



#include "flom_conns.h"
#include "flom_msg.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_RSRC



/**
 * Type of resource that must be locked
 */
typedef enum flom_rsrc_type_e {
    /**
     * Null resource type
     */
    FLOM_RSRC_TYPE_NULL,
    /**
     * Simple resource type (a single non numerical resource)
     */
    FLOM_RSRC_TYPE_SIMPLE,
    /**
     * Numeric resource type (a numerical resource)
     */
    FLOM_RSRC_TYPE_NUMERIC,
    /**
     * Set resource type (a resource set)
     */
    FLOM_RSRC_TYPE_SET,
    /**
     * Hierarchical resource type
     */
    FLOM_RSRC_TYPE_HIER,
    /**
     * Sequence resource type
     */
    FLOM_RSRC_TYPE_SEQUENCE,
    /**
     * Timestamp resource type
     */
    FLOM_RSRC_TYPE_TIMESTAMP,
    /**
     * Number of managed resource types
     */
    FLOM_RSRC_TYPE_N
} flom_rsrc_type_t;



/**
 * Lock/connection pair: used to store information related to the lock
 * requested by a connection (a client)
 */
struct flom_rsrc_conn_lock_s {
    /**
     * Information related to lock request
     */
    union {
        /**
         * Type of lock requested by the connection
         */
        flom_lock_mode_t            lock_mode;
        /**
         * Resource quantity requested by the connection (numeric resources)
         */
        gint                        quantity;
        /**
         * Sequence value assigned to the lock holder (sequence resources)
         */
        guint                       sequence_value;
        /**
         * Timestamp value assigned to the lock holder (timestamp resources)
         */
        struct timeval              timestamp_value;
    } info;
    /**
     * Resource name is necessary of hierarchical resources only because
     * the a resource is a tree of names
     */
    gchar                      *name;
    /**
     * Rollback flag is necessary for transactional resources like
     * sequence resource
     */
    int                         rollback;
    /**
     * Connection requesting the lock
     */
    flom_conn_t                *conn;
};



/**
 * Resource data for type "simple" @ref FLOM_RSRC_TYPE_SIMPLE
 */
struct flom_rsrc_data_simple_s {
    /**
     * List of connections with an acquired lock
     */
    GSList                 *holders;
    /**
     * List of connections waiting for a lock
     */
    GQueue                 *waitings;
};



/**
 * Resource data for type "numeric" @ref FLOM_RSRC_TYPE_NUMERIC
 */
struct flom_rsrc_data_numeric_s {
    /**
     * Total quantity for the resource
     */
    gint                    total_quantity;
    /**
     * Locked quantity for the resource
     */
    gint                    locked_quantity;
    /**
     * List of connections with an acquired lock
     */
    GSList                 *holders;
    /**
     * List of connections waiting for a lock
     */
    GQueue                 *waitings;
};



/**
 * Resource data for type "set" @ref FLOM_RSRC_TYPE_SET
 */
struct flom_rsrc_data_set_s {
    /**
     * Index of the next available member
     */
    guint                   index;
    /**
     * Array of elements in set; every element has a name and the connection
     * of the lock holder
     */
    GArray                 *elements;
    /**
     * List of connections waiting for an element to lock
     */
    GQueue                 *waitings;
};



/**
 * Every element of elements array in struct @ref flom_rsrc_data_set_s is
 * of this type
 */
struct flom_rsrc_data_set_element_s {
    /**
     * Name of the element
     */
    gchar                         *name;
    /**
     * Connection holding the lock on the element
     */
    flom_conn_t                   *conn;
};



/**
 * Every element of elements array (leaves) is of this struct type
 */
struct flom_rsrc_data_hier_element_s {
    /**
     * Name of the level
     */
    gchar                         *name;
    /**
     * List of connections with an acquired lock at this level
     */
    GSList                        *holders;
    /**
     * Next level elements of the same type of this one
     */
    GPtrArray                     *leaves;
};



/**
 * Resource data for type "hierarchical" @ref FLOM_RSRC_TYPE_HIER
 * It's a tree with undefined levels and undefined leaves for every level
 */
struct flom_rsrc_data_hier_s {
    /**
     * Root element, contains the first level of the resource name
     */
    struct flom_rsrc_data_hier_element_s  *root;
    /**
     * Queue of connections waiting for a lock
     */
    GQueue                                *waitings;
};



/**
 * Resource data for type "sequence" @ref FLOM_RSRC_TYPE_SEQUENCE
 */
struct flom_rsrc_data_sequence_s {
    /**
     * Total quantity for the resource
     */
    gint                    total_quantity;
    /**
     * Locked quantity for the resource
     */
    gint                    locked_quantity;
    /**
     * Next value that must be used for the sequence
     */
    guint                   next_value;
    /**
     * Sequence values that has been rolled back and must be re-used before
     * producing new ones
     */
    GQueue                 *rolled_back;
    /**
     * List of connections with an acquired lock
     */
    GSList                 *holders;
    /**
     * List of connections waiting for a lock
     */
    GQueue                 *waitings;
};



/**
 * Resource data for type "timestamp" @ref FLOM_RSRC_TYPE_TIMESTAMP
 */
struct flom_rsrc_data_timestamp_s {
    /**
     * Format for strftime function
     */
    gchar                  *format;
    /**
     * Minimum interval between two consecutive timestamps
     */
    struct timeval          interval;
    /**
     * Total quantity for the resource
     */
    gint                    total_quantity;
    /**
     * Locked quantity for the resource
     */
    gint                    locked_quantity;
    /**
     * Last supplied timestamp
     */
    struct timeval          last_timestamp;
    /**
     * List of connections with an acquired lock
     */
    GSList                 *holders;
    /**
     * List of connections waiting for a lock
     */
    GQueue                 *waitings;
};



/* necessary to declare flom_resource_t used inside the struct ("class")
   definition */
struct flom_resource_s;
/**
 * Resource type: a resource is an object that can be locked!
 */
typedef struct flom_resource_s flom_resource_t;
/**
 * Base struct for resource object
 */
struct flom_resource_s {
    /**
     * Resource type
     */
    flom_rsrc_type_t          type;
    /**
     * Resource name (allocated by g_strdup)
     */
    gchar                    *name;
    /**
     * Locking data associated with a resource (it depends from type)
     */
    union {
        struct flom_rsrc_data_simple_s       simple;
        struct flom_rsrc_data_numeric_s      numeric;
        struct flom_rsrc_data_set_s          set;
        struct flom_rsrc_data_hier_s         hier;
        struct flom_rsrc_data_sequence_s     sequence;
        struct flom_rsrc_data_timestamp_s    timestamp;
    } data;
    /**
     * Method called to initialize a new resource
     */
    int   (*init)      (flom_resource_t *resource, const gchar *name);
    /**
     * Method called to process incoming messages (it depends from resource
     * type)
     */
    int   (*inmsg)     (flom_resource_t *, flom_uid_t, flom_conn_t *,
                        struct flom_msg_s *, struct timeval *next_deadline);
    /**
     * Method called to process a clean-up signal (client disconnected)
     */
    int   (*clean)     (flom_resource_t *, flom_uid_t, flom_conn_t *);
    /**
     * Method called to clean-up the entire resource (it's the destructor)
     */
    void  (*free)      (flom_resource_t *);
    /**
     * Method called when poll exits due to time-out
     */
    int   (*timeout)   (flom_resource_t *, flom_uid_t, struct timeval *next_deadline);
    /**
     * Method called to compare the name of the current managed resource with
     * an external supplied name
     */
    int   (*compare_name)   (const flom_resource_t *, const gchar *);
};



/**
 * This is a global static object shared by all modules and contain the
 * precompiled regular expression used to parse resource names and check if
 * they are valid resource names
 */
extern regex_t global_res_name_preg[];



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


    
    /**
     * Initialize the precompiled regular expression @ref global_res_name_preg
     * @return a reason code
     */
    int global_res_name_preg_init();
    
    

    /**
     * Free the precompiled regular expression @ref global_res_name_preg
     */
    void global_res_name_preg_free();
    
    

    /**
     * Retrieve the type of the resource from its name
     * @param resource_name IN resource name
     * @return resource type @ref flom_rsrc_type_t;
     *     @ref FLOM_RSRC_TYPE_NULL means the name is not valid for
     *      any resource type
     */
    flom_rsrc_type_t flom_rsrc_get_type(const gchar *resource_name);



    /**
     * Retrieve a human readable representation for a resource type (a
     * string)
     * @param res_type IN resource type
     * @return a string
     */
    const gchar *flom_rsrc_get_type_human_readable(flom_rsrc_type_t res_type);

    
    
    /**
     * Check if the resource is transactional from its name
     * @param resource_name IN resource name
     * @return a boolean value: TRUE, the resource supports transactions
     */
    int flom_rsrc_get_transactional(const gchar *resource_name);

    

    /**
     * Retrieve the quantity associated to a numeric resource
     * @param resource_name IN resource name
     * @param type IN resource type
     * @param number OUT quantity
     * @return a reason code
     */
    int flom_rsrc_get_number(const gchar *resource_name, flom_rsrc_type_t type,
                             gint *number);



    /**
     * Retrieve the infix part of a resource name
     * @param resource_name IN resource name
     * @param type IN resource type
     * @param infix OUT a NULL value in case of error, or an allocated string
     *        that MUST be released with g_free
     * @return a reason code
     */
    int flom_rsrc_get_infix(const gchar *resource_name, flom_rsrc_type_t tpye,
                            gchar **infix);



    /**
     * Split a resource set name in to distinct elements
     * @param resource_name IN resource name
     * @param elements OUT array of elements
     * @return a reason code
     */
    int flom_rsrc_get_elements(const gchar *resource_name, GArray *elements);



    /**
     * Create a resource connection lock struct
     * @return a valid pointer or NULL
     */
    struct flom_rsrc_conn_lock_s *flom_rsrc_conn_lock_new(void);

    

    /**
     * Release/free/delete a resource connection lock struct
     * @param frcl OUT reference to a resource connection lock struct
     */
    void flom_rsrc_conn_lock_delete(struct flom_rsrc_conn_lock_s *frcl);



    /**
     * Find in holders list the element related to a connection
     * @param holders IN the list with lock holder elements
     * @param conn IN the connection that must be found
     * @return the element related to the searched connection or NULL
     */
    GSList *flom_rsrc_conn_find(GSList *holders, flom_conn_t *conn);

    

    /**
     * Initialize a resource
     * @param resource IN/OUT object to be initialized
     * @param type IN resource type
     * @param name IN resource name
     * @return a reason code
     */
    int flom_resource_init(flom_resource_t *resource,
                           flom_rsrc_type_t type, const gchar *name);



    /**
     * Free all dynamically allocated memory
     * @param resource IN/OUT object to release
     */
    void flom_resource_free(flom_resource_t *resource);



    /**
     * Compare the name of the current resource and an external name passed
     * to the method
     * @param resource IN reference to this resource object
     * @param name IN another resource name to compare with this resource name
     * @return -1,0,+1 as strcmp
     */
    int flom_resource_compare_name(const flom_resource_t *resource,
                                   const gchar *name);



    /**
     * Default timeout callback function: it does nothing
     * @param resource IN reference to this resource object
     * @param locker_uid IN unique identifier or the locker that's managing
     *        the resource
     * @param next_deadline OUT next deadline for the first timestamp that can
     *        be generated
     * @return FLOM_RC_OK
     */
    int flom_resource_timeout(flom_resource_t *resource,
                              flom_uid_t locker_uid,
                              struct timeval *next_deadline);

    
    
    /**
     * Get the name of a resource
     * @param resource IN referente to resource object
     * @return the name of the resource
     */
    static inline const gchar *flom_resource_get_name(
        const flom_resource_t *resource) {
        return resource->name;
    }



    /**
     * Get the type of a resource
     * @param resource IN referente to resource object
     * @return the type of the resource
     */
    static inline flom_rsrc_type_t flom_resource_get_type(
        const flom_resource_t *resource) {
        return resource->type;
    }

    

#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_RSRC_H */
