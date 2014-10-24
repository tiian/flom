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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include "flom.h"
#include "flom_config.h"
#include "flom_conns.h"
#include "flom_client.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_API



/**
 * Mutex used to access flom initialization flag
 */
GStaticMutex flom_init_mutex = G_STATIC_MUTEX_INIT;
/**
 * Flom library is initialized
 */
int flom_init_flag = FALSE;



/**
 * Check flom library is initialized
 */
int flom_init_check(void);



int flom_handle_init(flom_handle_t *handle)
{
    enum Exception { G_TRY_MALLOC_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_handle_init\n"));
    TRY {
        /* memory reset */
        memset(handle, 0, sizeof(flom_handle_t));
        /* allocate memory for connection data structure */
        if (NULL == (handle->conn_data = g_try_malloc0(
                         sizeof(struct flom_conn_data_s))))
            THROW(G_TRY_MALLOC_ERROR);
        /* state reset */
        handle->state = FLOM_HANDLE_STATE_INIT;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_handle_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



flom_handle_t *flom_handle_new(void)
{
    flom_handle_t *tmp_handle = NULL;

    /* dummy loop */
    while (TRUE) {
        int ret_cod;
        
        /* check flom library is initialized */
        if (FLOM_RC_OK != flom_init_check())
            break;
        FLOM_TRACE(("flom_handle_new\n"));
        /* try to allocate a new handle object */
        if (NULL == (tmp_handle = (flom_handle_t *)g_try_malloc(
                         sizeof(flom_handle_t)))) {
            FLOM_TRACE(("flom_handle_new: g_try_malloc returned NULL\n"));
            break;
        }
        /* initialize the new handle */
        if (FLOM_RC_OK != (ret_cod = flom_handle_init(tmp_handle))) {
            FLOM_TRACE(("flom_handle_new: flom_handle_init returned %d\n",
                        ret_cod));
            g_free(tmp_handle);
            tmp_handle = NULL;
            break;
        }
        /* exit the loop after one cycle */
        break;
    } /* while (TRUE) */
    FLOM_TRACE(("flom_handle_new: return %p\n", tmp_handle));
    return tmp_handle;
}



int flom_handle_clean(flom_handle_t *handle)
{
    enum Exception { API_INVALID_SEQUENCE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_handle_clean\n"));
    TRY {
        /* check handle state */
        if (FLOM_HANDLE_STATE_INIT != handle->state &&
            FLOM_HANDLE_STATE_DISCONNECTED != handle->state) {
            FLOM_TRACE(("flom_handle_clean: handle->state=%d\n",
                        handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* release memory of connection data structure */
        g_free(handle->conn_data);
        handle->conn_data = NULL;
        /* clean handle state */
        handle->state = FLOM_HANDLE_STATE_CLEANED;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case API_INVALID_SEQUENCE:
                ret_cod = FLOM_RC_API_INVALID_SEQUENCE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_handle_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_handle_delete(flom_handle_t *handle)
{
    /* dummy loop */
    while (TRUE) {
        int ret_cod;
        
        /* check flom library is initialized */
        if (FLOM_RC_OK != flom_init_check())
            break;
        FLOM_TRACE(("flom_handle_delete: handle=%p\n", handle));
        /* check handle is not NULL */
        if (NULL == handle) {
            FLOM_TRACE(("flom_handle_delete: handle is null, skipping...\n"));
            break;
        }
        /* clean object handle */
        if (FLOM_RC_OK != (ret_cod = flom_handle_clean(handle))) {
            FLOM_TRACE(("flom_handle_new: flom_handle_clean returned %d, "
                        "ignoring it and going on...\n", ret_cod));
        }
        /* reset the object handle to prevent misuse of the associated
           memory */
        memset(handle, 0, sizeof(flom_handle_t));
        /* remove object handle */
        g_free(handle);
        /* exit the loop after one cycle */
        break;
    } /* while (TRUE) */
    FLOM_TRACE(("flom_handle_delete: exiting\n"));
}


int flom_lock(flom_handle_t *handle, char *element, size_t element_size)
{
    enum Exception { API_INVALID_SEQUENCE
                     , NULL_OBJECT
                     , CLIENT_CONNECT_ERROR
                     , CLIENT_LOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_lock\n"));
    TRY {
        struct flom_conn_data_s *cd =
            (struct flom_conn_data_s *)handle->conn_data;
        /* check handle state */
        if (FLOM_HANDLE_STATE_INIT != handle->state &&
            FLOM_HANDLE_STATE_CONNECTED != handle->state &&
            FLOM_HANDLE_STATE_DISCONNECTED != handle->state) {
            FLOM_TRACE(("flom_lock: handle->state=%d\n", handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* check the connection data pointer is not NULL (we can't be sure
           it's a valid pointer) */
        if (NULL == handle->conn_data)
            THROW(NULL_OBJECT);
        /* open a connection to a valid lock manager */
        if (FLOM_HANDLE_STATE_CONNECTED != handle->state) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect(cd, TRUE)))
                THROW(CLIENT_CONNECT_ERROR);
            /* state update */
            handle->state = FLOM_HANDLE_STATE_CONNECTED;
        } else {
            FLOM_TRACE(("flom_lock: handle already connected (%d), "
                        "skipping...\n", handle->state));
        }
        /* lock acquisition */
        if (FLOM_RC_OK != (ret_cod = flom_client_lock(
                               cd, flom_config_get_resource_timeout(),
                               element, element_size)))
            THROW(CLIENT_LOCK_ERROR);
        /* state update */
        handle->state = FLOM_HANDLE_STATE_LOCKED;

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case API_INVALID_SEQUENCE:
                ret_cod = FLOM_RC_API_INVALID_SEQUENCE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CLIENT_CONNECT_ERROR:
            case CLIENT_LOCK_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_unlock(flom_handle_t *handle)
{
    enum Exception { API_INVALID_SEQUENCE
                     , NULL_OBJECT
                     , CLIENT_UNLOCK_ERROR
                     , CLIENT_DISCONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_unlock\n"));
    TRY {
        struct flom_conn_data_s *cd =
            (struct flom_conn_data_s *)handle->conn_data;
        /* check handle state */
        if (FLOM_HANDLE_STATE_LOCKED != handle->state &&
            FLOM_HANDLE_STATE_CONNECTED != handle->state) {
            FLOM_TRACE(("flom_unlock: handle->state=%d\n", handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* check the connection data pointer is not NULL (we can't be sure
           it's a valid pointer) */
        if (NULL == handle->conn_data)
            THROW(NULL_OBJECT);
        if (FLOM_HANDLE_STATE_LOCKED == handle->state) {
            /* lock release */
            if (FLOM_RC_OK != (ret_cod = flom_client_unlock(cd)))
                THROW(CLIENT_UNLOCK_ERROR);
            /* state update */
            handle->state = FLOM_HANDLE_STATE_CONNECTED;
        } else {
            FLOM_TRACE(("flom_unlock: resource already unlocked (%d), "
                        "skipping...\n", handle->state));
        }
        /* gracefully disconnect from daemon */
        if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(cd)))
            THROW(CLIENT_DISCONNECT_ERROR);
        /* state update */
        handle->state = FLOM_HANDLE_STATE_DISCONNECTED;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case API_INVALID_SEQUENCE:
                ret_cod = FLOM_RC_API_INVALID_SEQUENCE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CLIENT_UNLOCK_ERROR:
            case CLIENT_DISCONNECT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_unlock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



/*
 * Internal library methods not intended for application development
 */



int flom_init_check(void)
{
    int mutex_locked = FALSE;
    int ret_cod = FLOM_RC_OK;
    
    /* initialize only if not already initialized! */
    if (!flom_init_flag) {
        /* this is a synchronization hole... see below */
        /* synchronize this critical section */
        g_static_mutex_lock(&flom_init_mutex);
        mutex_locked = TRUE;
        /* dummy loop, necessary because I need break instruction ;) */
        while (TRUE) {
            /* stop if another thread performed initialization in the
               during synchronization hole (unlikely but not impossible) */
            if (flom_init_flag)
                break;
            /* initialize trace component */
            FLOM_TRACE_INIT;
            /* initialize regular expression table */
            if (FLOM_RC_OK != (ret_cod = global_res_name_preg_init()))
                break;
            /* reset global configuration */
            flom_config_reset();
            /* initialize configuration with standard system, standard
               user and user customized config files */
            if (FLOM_RC_OK != (ret_cod = flom_config_init(NULL)))
                break;
            if (NULL != flom_config_get_command_trace_file())
                /* change trace destination if necessary */
                FLOM_TRACE_REOPEN(flom_config_get_command_trace_file());
            /* check configuration */
            if (FLOM_RC_OK != (ret_cod = flom_config_check()))
                break;
            /* initialization completed */
            flom_init_flag = TRUE;
            /* exit loop after exactly one cycle */
            break; 
        } /* while (TRUE) */

    } /* if (!flom_init_flag) */
    
    if (mutex_locked)
        g_static_mutex_unlock(&flom_init_mutex);
    return ret_cod;
}
