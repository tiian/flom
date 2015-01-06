/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_client.h"
#include "flom_errors.h"
#include "flom_handle.h"
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
    enum Exception { G_TRY_MALLOC_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , CONFIG_INIT_ERROR
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
            THROW(G_TRY_MALLOC_ERROR1);
        /* allocate memory for configuration data structure */
        if (NULL == (handle->config = g_try_malloc0(
                         sizeof(flom_config_t))))
            THROW(G_TRY_MALLOC_ERROR2);
        /* initialize config object */
        if (FLOM_RC_OK != (ret_cod = flom_config_clone(handle->config)))
            THROW(CONFIG_INIT_ERROR);
        /* clients can not fork a new flom daemon: this restriction is
         * necessary to avoid the side effects related to daemonization that
         * can affect a general purpose environment... This is a CLIENT!!! */
        flom_config_set_lifespan(handle->config, 0);
        /* state reset */
        handle->state = FLOM_HANDLE_STATE_INIT;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR1:
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case CONFIG_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* clean memory if an error occurred */
    if (NONE != excp) {
        if (NULL != handle->config) {
            flom_config_free(handle->config);
            g_free(handle->config);
            handle->config = NULL;
        } /* if (NULL != handle->config) */
        if (NULL != handle->conn_data) {
            g_free(handle->conn_data);
            handle->conn_data = NULL;
        } /* if (NULL != handle->conn_data) */
    } /* if (NONE != excp) */
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
        FLOM_TRACE(("flom_handle_new: allocated handle %p\n", tmp_handle));
        /* initialize the new handle */
        if (FLOM_RC_OK != (ret_cod = flom_handle_init(tmp_handle))) {
            FLOM_TRACE(("flom_handle_new: flom_handle_init returned %d\n",
                        ret_cod));
            g_free(tmp_handle);
            FLOM_TRACE(("flom_handle_new: deallocated handle %p\n",
                        tmp_handle));
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
    enum Exception { FLOM_HANDLE_UNLOCK_ERROR
                     , API_INVALID_SEQUENCE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_handle_clean\n"));
    TRY {
        /* is the handle locked? we must unlock it before going on... */
        if (FLOM_HANDLE_STATE_LOCKED == handle->state) {
            if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(handle)))
                THROW(FLOM_HANDLE_UNLOCK_ERROR);
        }
        /* check handle state */
        if (FLOM_HANDLE_STATE_INIT != handle->state &&
            FLOM_HANDLE_STATE_DISCONNECTED != handle->state) {
            FLOM_TRACE(("flom_handle_clean: handle->state=%d\n",
                        handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* release memory allocated for configuration object */
        flom_config_free(handle->config);
        g_free(handle->config);
        handle->config = NULL;
        /* release memory of connection data structure */
        g_free(handle->conn_data);
        handle->conn_data = NULL;
        /* clean handle state */
        handle->state = FLOM_HANDLE_STATE_CLEANED;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case FLOM_HANDLE_UNLOCK_ERROR:
                break;
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
        FLOM_TRACE(("flom_handle_new: deallocated handle %p\n", handle));
        /* exit the loop after one cycle */
        break;
    } /* while (TRUE) */
    FLOM_TRACE(("flom_handle_delete: exiting\n"));
}


int flom_handle_lock(flom_handle_t *handle, char *element, size_t element_size)
{
    enum Exception { NULL_OBJECT
                     , API_INVALID_SEQUENCE
                     , OBJ_CORRUPTED
                     , CLIENT_CONNECT_ERROR
                     , CLIENT_LOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_handle_lock\n"));
    TRY {
        struct flom_conn_data_s *cd = NULL;
        /* check handle is not NULL */
        if (NULL == handle)
            THROW(NULL_OBJECT);
        /* cast and retrieve conn_data fron the proxy object */
        cd = (struct flom_conn_data_s *)handle->conn_data;
        /* check handle state */
        if (FLOM_HANDLE_STATE_INIT != handle->state &&
            FLOM_HANDLE_STATE_CONNECTED != handle->state &&
            FLOM_HANDLE_STATE_DISCONNECTED != handle->state) {
            FLOM_TRACE(("flom_handle_lock: handle->state=%d\n",
                        handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* check the connection data pointer is not NULL (we can't be sure
           it's a valid pointer) */
        if (NULL == handle->conn_data)
            THROW(OBJ_CORRUPTED);
        /* open a connection to a valid lock manager */
        if (FLOM_HANDLE_STATE_CONNECTED != handle->state) {
            if (FLOM_RC_OK != (ret_cod = flom_client_connect(
                                   handle->config, cd, TRUE)))
                THROW(CLIENT_CONNECT_ERROR);
            /* state update */
            handle->state = FLOM_HANDLE_STATE_CONNECTED;
        } else {
            FLOM_TRACE(("flom_handle_lock: handle already connected (%d), "
                        "skipping...\n", handle->state));
        }
        /* lock acquisition */
        if (FLOM_RC_OK != (ret_cod = flom_client_lock(
                               handle->config, cd,
                               flom_config_get_resource_timeout(
                                   handle->config),
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
            case OBJ_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
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
    FLOM_TRACE(("flom_handle_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_handle_unlock(flom_handle_t *handle)
{
    enum Exception { NULL_OBJECT
                     , API_INVALID_SEQUENCE
                     , OBJ_CORRUPTED
                     , CLIENT_UNLOCK_ERROR
                     , CLIENT_DISCONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_handle_unlock\n"));
    TRY {
        struct flom_conn_data_s *cd = NULL;
        /* check handle is not NULL */
        if (NULL == handle)
            THROW(NULL_OBJECT);
        /* cast and retrieve conn_data fron the proxy object */
        cd = (struct flom_conn_data_s *)handle->conn_data;
        /* check handle state */
        if (FLOM_HANDLE_STATE_LOCKED != handle->state &&
            FLOM_HANDLE_STATE_CONNECTED != handle->state) {
            FLOM_TRACE(("flom_handle_unlock: handle->state=%d\n",
                        handle->state));
            THROW(API_INVALID_SEQUENCE);
        }
        /* check the connection data pointer is not NULL (we can't be sure
           it's a valid pointer) */
        if (NULL == handle->conn_data)
            THROW(OBJ_CORRUPTED);
        if (FLOM_HANDLE_STATE_LOCKED == handle->state) {
            /* lock release */
            if (FLOM_RC_OK != (ret_cod = flom_client_unlock(
                                   handle->config, cd)))
                THROW(CLIENT_UNLOCK_ERROR);
            /* state update */
            handle->state = FLOM_HANDLE_STATE_CONNECTED;
        } else {
            FLOM_TRACE(("flom_handle_unlock: resource already unlocked (%d), "
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
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case API_INVALID_SEQUENCE:
                ret_cod = FLOM_RC_API_INVALID_SEQUENCE;
                break;
            case OBJ_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
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
    FLOM_TRACE(("flom_handle_unlock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



/*
 * Getter/setter methods to manage config
 */



int flom_handle_get_discovery_attempts(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_discovery_attempts: value=%d\n",
                flom_config_get_discovery_attempts(handle->config)));
    return (int)flom_config_get_discovery_attempts(handle->config);
}



void flom_handle_set_discovery_attempts(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_discovery_attempts: "
                "old value=%d, new value=%d\n",
                flom_config_get_discovery_attempts(handle->config), value));
    flom_config_set_discovery_attempts(handle->config, (gint)value);
}



int flom_handle_get_discovery_timeout(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_discovery_timeout: value=%d\n",
                flom_config_get_discovery_timeout(handle->config)));
    return (int)flom_config_get_discovery_timeout(handle->config);
}



void flom_handle_set_discovery_timeout(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_discovery_timeout: "
                "old value=%d, new value=%d\n",
                flom_config_get_discovery_timeout(handle->config), value));
    flom_config_set_discovery_timeout(handle->config, (gint)value);
}



int flom_handle_get_discovery_ttl(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_discovery_ttl: value=%d\n",
                flom_config_get_discovery_ttl(handle->config)));
    return (int)flom_config_get_discovery_ttl(handle->config);
}



void flom_handle_set_discovery_ttl(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_discovery_ttl: "
                "old value=%d, new value=%d\n",
                flom_config_get_discovery_ttl(handle->config), value));
    flom_config_set_discovery_ttl(handle->config, (gint)value);
}



flom_lock_mode_t flom_handle_get_lock_mode(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_lock_mode: value=%d\n",
                flom_config_get_lock_mode(handle->config)));
    return flom_config_get_lock_mode(handle->config);
}



void flom_handle_set_lock_mode(flom_handle_t *handle, flom_lock_mode_t value)
{
    FLOM_TRACE(("flom_handle_set_lock_mode: "
                "old value=%d, new value=%d\n",
                flom_config_get_lock_mode(handle->config), value));
    flom_config_set_lock_mode(handle->config, value);
}



const char *flom_handle_get_multicast_address(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_multicast_address: value='%s'\n",
                STRORNULL(flom_config_get_multicast_address(handle->config))));
    return (const char *)flom_config_get_multicast_address(handle->config);
}



void flom_handle_set_multicast_address(flom_handle_t *handle,
                                       const char *value)
{
    FLOM_TRACE(("flom_handle_set_multicast_address: "
                "old value='%s', new value='%s'\n",
                STRORNULL(flom_config_get_multicast_address(handle->config)),
                STRORNULL(value)));
    flom_config_set_multicast_address(handle->config, (const gchar *)value);
    /* reset socket name and unicast address */
    if (NULL != value) {
        flom_handle_set_socket_name(handle, NULL);
        flom_handle_set_unicast_address(handle, NULL);
    } /* if (NULL != value) */
}



int flom_handle_get_multicast_port(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_multicast_port: value=%d\n",
                flom_config_get_multicast_port(handle->config)));
    return (int)flom_config_get_multicast_port(handle->config);
}



void flom_handle_set_multicast_port(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_multicast_port: "
                "old value=%d, new value=%d\n",
                flom_config_get_multicast_port(handle->config), value));
    flom_config_set_multicast_port(handle->config, (gint)value);
}



int flom_handle_get_resource_create(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_resource_create: value=%d\n",
                flom_config_get_resource_create(handle->config)));
    return flom_config_get_resource_create(handle->config);
}



void flom_handle_set_resource_create(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_resource_create: "
                "old value=%d, new value=%d\n",
                flom_config_get_resource_create(handle->config), value));
    return flom_config_set_resource_create(handle->config, value);
}



int flom_handle_get_resource_idle_lifespan(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_resource_idle_lifespan: value=%d\n",
                flom_config_get_resource_idle_lifespan(handle->config)));
    return (int)flom_config_get_resource_idle_lifespan(handle->config);
}



void flom_handle_set_resource_idle_lifespan(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_resource_idle_lifespan: "
                "old value=%d, new value=%d\n",
                flom_config_get_resource_idle_lifespan(handle->config),
                value));
    flom_config_set_resource_idle_lifespan(handle->config, (gint)value);
}



const char *flom_handle_get_resource_name(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_resource_name: value='%s'\n",
                STRORNULL(flom_config_get_resource_name(handle->config))));
    return (const char *)flom_config_get_resource_name(handle->config);
}



int flom_handle_set_resource_name(flom_handle_t *handle, const char *value)
{
    FLOM_TRACE(("flom_handle_set_resource_name: "
                "old value='%s', new value='%s'\n",
                STRORNULL(flom_config_get_resource_name(handle->config)),
                STRORNULL(value)));
    return flom_config_set_resource_name(handle->config, (const gchar *)value);
}



int flom_handle_get_resource_quantity(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_resource_quantity: value=%d\n",
                flom_config_get_resource_quantity(handle->config)));
    return (int)flom_config_get_resource_quantity(handle->config);
}



void flom_handle_set_resource_quantity(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_resource_quantity: "
                "old value=%d, new value=%d\n",
                flom_config_get_resource_quantity(handle->config), value));
    flom_config_set_resource_quantity(handle->config, (int)value);
}



int flom_handle_get_resource_timeout(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_resource_timeout: value=%d\n",
                flom_config_get_resource_timeout(handle->config)));
    return (int)flom_config_get_resource_timeout(handle->config);
}



void flom_handle_set_resource_timeout(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_resource_timeout: "
                "old value=%d, new value=%d\n",
                flom_config_get_resource_timeout(handle->config), value));
    return flom_config_set_resource_timeout(handle->config, (int)value);
}



const char *flom_handle_get_socket_name(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_socket_name: value='%s'\n",
                STRORNULL(flom_config_get_socket_name(handle->config))));
    return (const char *)flom_config_get_socket_name(handle->config);
}



int flom_handle_set_socket_name(flom_handle_t *handle, const char *value)
{
    FLOM_TRACE(("flom_handle_set_socket_name: "
                "old value='%s', new value='%s'\n",
                STRORNULL(flom_config_get_socket_name(handle->config)),
                STRORNULL(value)));
    /* reset unicast and multicast addresses */
    if (NULL != value) {
        flom_handle_set_unicast_address(handle, NULL);
        flom_handle_set_multicast_address(handle, NULL);
    } /* if (NULL != value) */
    return flom_config_set_socket_name(handle->config, (const gchar *)value);
}



const char *flom_handle_get_trace_filename(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_trace_filename: value='%s'\n",
                STRORNULL(flom_config_get_command_trace_file(
                              handle->config))));
    return (const char *)flom_config_get_command_trace_file(handle->config);
}



void flom_handle_set_trace_filename(flom_handle_t *handle, const char *value)
{
    FLOM_TRACE(("flom_handle_set_trace_filename: "
                "old value='%s', new value='%s'\n",
                STRORNULL(flom_config_get_command_trace_file(handle->config)),
                STRORNULL(value)));
    flom_config_set_command_trace_file(handle->config, (const gchar *)value);
}



const char *flom_handle_get_unicast_address(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_unicast_address: value='%s'\n",
                STRORNULL(flom_config_get_unicast_address(handle->config))));
    return (const char *)flom_config_get_unicast_address(handle->config);
}



void flom_handle_set_unicast_address(flom_handle_t *handle, const char *value)
{
    FLOM_TRACE(("flom_handle_set_unicast_address: "
                "old value='%s', new value='%s'\n",
                STRORNULL(flom_config_get_unicast_address(handle->config)),
                STRORNULL(value)));
    flom_config_set_unicast_address(handle->config, (const gchar *)value);
    /* reset socket name and multicast address*/
    if (NULL != value) {
        flom_handle_set_socket_name(handle, NULL);
        flom_handle_set_multicast_address(handle, NULL);
    } /* if (NULL != value) */
}



int flom_handle_get_unicast_port(const flom_handle_t *handle)
{
    FLOM_TRACE(("flom_handle_get_unicast_port: value=%d\n",
                flom_config_get_unicast_port(handle->config)));
    return (int)flom_config_get_unicast_port(handle->config);
}



void flom_handle_set_unicast_port(flom_handle_t *handle, int value)
{
    FLOM_TRACE(("flom_handle_set_unicast_port: "
                "old value=%d, new value=%d\n",
                flom_config_get_unicast_port(handle->config), value));
    flom_config_set_unicast_port(handle->config, (gint)value);
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
            flom_config_reset(NULL);
            /* initialize configuration with standard system, standard
               user and user customized config files */
            if (FLOM_RC_OK != (ret_cod = flom_config_init(NULL, NULL)))
                break;
            if (NULL != flom_config_get_command_trace_file(NULL))
                /* change trace destination if necessary */
                FLOM_TRACE_REOPEN(flom_config_get_command_trace_file(NULL),
                                  flom_config_get_append_trace_file(NULL));
            /* check configuration */
            if (FLOM_RC_OK != (ret_cod = flom_config_check(NULL)))
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
