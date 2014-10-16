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



int flom_lock()
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_lock\n"));
    TRY {
        /* @@@ restart from here */
            
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
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



int flom_unlock()
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* check flom library is initialized */
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("flom_unlock\n"));
    TRY {
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
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
            /* exit loop after exactly one cycle */
            break; 
        } /* while (TRUE) */

    } /* if (!flom_init_flag) */
    
    if (mutex_locked)
        g_static_mutex_unlock(&flom_init_mutex);
    return ret_cod;
}
