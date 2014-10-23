/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_H
# define FLOM_H



#include <glib.h>



#include "flom_errors.h"



/**
 * This scalar type is used to represent the state of an handle
 */
typedef enum flom_handle_state_e {
    /**
     * Initial state
     */
    FLOM_HANDLE_STATE_INIT,
    /**
     * The client is connected to the daemon and the resource is NOT locked
     */
    FLOM_HANDLE_STATE_CONNECTED,
    /**
     * The client is connected to the daemon and the resource is locked
     */
    FLOM_HANDLE_STATE_LOCKED,
    /**
     * The client is NOT connected to the daemon
     */
    FLOM_HANDLE_STATE_DISCONNECTED,
    /**
     * The handle memory was released and the handle itself can NOT be used
     * without a call to @ref flom_handle_init method
     */
    FLOM_HANDLE_STATE_CLEANED
} flom_handle_state_t;



/**
 * This object is used to save all the necessary context to interact with
 * libflom library.
 * Some fields use "void *" type to avoid useless internal details exposure
 * (flom methods proxies the correct types)
 */
typedef struct flom_handle_s {
    /**
     * Handle state
     */
    flom_handle_state_t   state;
    /**
     * Connection data
     */
    gpointer              conn_data;
} flom_handle_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* @@@
 * introduce methods: flom_handle_new and flom_handle_delete that use
 * dynamic memory instead of pre-allocated structures.
 *
 * create API case tests, stressing the protocol sequence (handle->state)
 *
 * move configuration from "global" to "local"
 */
    
    /**
     * Initialize an object handle; this function MUST be called before the
     * first usage of a new handle or after an handle has been cleaned up with
     * @ref flom_handle_clean
     * @param handle IN/OUT the object to initialize
     * @return a reason code
     */
    int flom_handle_init(flom_handle_t *handle);
    


    /**
     * Clean an object handle; this function MUST be called before the out of
     * scope of an handle; if this method is not called a memory leak will
     * be generated. For every object initialized with @ref flom_handle_init
     * there must be a call to this method.
     * @param handle IN/OUT the object to initialize
     * @return a reason code
     */
    int flom_handle_clean(flom_handle_t *handle);
    


    /**
     * Lock a resource
     * @param handle IN/OUT library handle
     * @param element OUT contains the name of the locked element if the
     *        resource is a resource set; set it to NULL if you are not
     *        interested in it
     * @param element_size IN maximum number of characters (null terminator
     *        included) that can be used by the function to store the name
     *        of the locked element
     * @return a reason code
     */
    int flom_lock(flom_handle_t *handle, char *element, size_t element_size);



    /**
     * Unlock a resource
     * @param handle IN/OUT library handle
     * @return a reason code
     */
    int flom_unlock(flom_handle_t *handle);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_H */
