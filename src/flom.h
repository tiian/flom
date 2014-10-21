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
 * This object is used to save all the necessary context to interact with
 * libflom library.
 * Some fields use "void *" type to avoid useless internal details exposure
 * (flom methods proxies the correct types)
 */
typedef struct flom_handle_s {
    /**
     * Connection data
     */
    gpointer   conn_data;
} flom_handle_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



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
     * @return a reason code
     */
    int flom_lock(flom_handle_t *handle);



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
