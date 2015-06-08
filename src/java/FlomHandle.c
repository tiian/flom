/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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



#include <jni.h>
#include <stdio.h>
#include "flom_defines.h"
#include "flom_trace.h"
#include "flom.h"
#include "FlomHandle.h"
 


/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_API



/* Allocate a new FlomHandle (native struct) */
JNIEXPORT jobject JNICALL Java_org_tiian_flom_FlomHandle_newFlomHandle(
    JNIEnv *env, jobject this_obj)
{
    enum Exception { MALLOC_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    jobject bb = NULL;

    if (FLOM_RC_OK != flom_init_check())
        return NULL;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFlomHandle\n"));
    TRY {
        flom_handle_t *h = flom_handle_new();
        jobject bb = (*env)->NewDirectByteBuffer(env, (void *)h,
                                                 NULL != h ? 
                                                 sizeof(flom_handle_t) :
                                                 0);
        if (NULL == h)
            THROW(MALLOC_ERROR);
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MALLOC_ERROR:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFlomHandle/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return bb;
}



JNIEXPORT void JNICALL Java_org_tiian_flom_FlomHandle_deleteFlomHandle
  (JNIEnv *env, jobject this_obj, jobject byte_buffer)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    if (FLOM_RC_OK != flom_init_check())
        return;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFlomHandle\n"));
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
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFlomHandle/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return;
}
