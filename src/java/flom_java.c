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
#include "flom_java.h"
 


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
    jobject byte_buffer = NULL;

    if (FLOM_RC_OK != flom_init_check())
        return NULL;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFlomHandle\n"));
    TRY {
        flom_handle_t *h = flom_handle_new();
        if (NULL == h)
            THROW(MALLOC_ERROR);
        byte_buffer = (*env)->NewDirectByteBuffer(env, (void *)h,
                                                  NULL != h ? 
                                                  sizeof(flom_handle_t) :
                                                  0);
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
    return byte_buffer;
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_deleteFlomHandle
  (JNIEnv *env, jobject this_obj)
{
    enum Exception { GET_OBJECT_CLASS_ERROR
                     , GET_FIELD_ID_ERROR
                     , NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    if (FLOM_RC_OK != flom_init_check())
        return;

    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFlomHandle\n"));
    TRY {
        jclass this_class;
        jfieldID field_id;
        jobject byte_buffer;
        flom_handle_t *fh;
        
        /* get a reference to this object's class */
        if (NULL == (this_class = (*env)->GetObjectClass(env, this_obj))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomException_deleteFlomHandle: "
                        "this_class == NULL\n"));
            THROW(GET_OBJECT_CLASS_ERROR);
        }
        /* get the field identificator */
        if (NULL == (field_id = (*env)->GetFieldID(
                         env, this_class, "NativeHandler",
                         "Ljava/nio/ByteBuffer;"))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomException_deleteFlomHandle: "
                        "field_id == NULL\n"));
            THROW(GET_FIELD_ID_ERROR);
        }
        /* get ByteBuffer reference */
        byte_buffer = (*env)->GetObjectField(env, this_obj, field_id);
        /* cast to flom_handle_t */
        if (NULL == (fh = (flom_handle_t *)(*env)->GetDirectBufferAddress(
                         env, byte_buffer)))
            THROW(NULL_OBJECT);
        flom_handle_delete(fh);
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GET_OBJECT_CLASS_ERROR:
                ret_cod = FLOM_RC_GET_OBJECT_CLASS_ERROR;
                break;
            case GET_FIELD_ID_ERROR:
                ret_cod = FLOM_RC_GET_FIELD_ID_ERROR;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFlomHandle/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return (jint)ret_cod;
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomException_getReturnCodeText
(JNIEnv *env, jobject this_obj)
{
    jstring return_string = NULL;
    jclass this_class;
    jfieldID field_id;
    jint return_code;
    
    /* get a reference to this object's class */
    if (NULL == (this_class = (*env)->GetObjectClass(env, this_obj))) {
        FLOM_TRACE(("Java_org_tiian_flom_FlomException_getReturnCodeText: "
                    "this_class == NULL\n"));
        return return_string;
    }
    /* get the field identificator */
    if (NULL == (field_id = (*env)->GetFieldID(env, this_class,
                                               "ReturnCode", "I"))) {
        FLOM_TRACE(("Java_org_tiian_flom_FlomException_getReturnCodeText: "
                    "field_id == NULL\n"));
        return return_string;
    }
    /* get ReturnCode value */
    return_code = (*env)->GetIntField(env, this_obj, field_id);
    /* get return string using native method */
    return_string = (*env)->NewStringUTF(env, flom_strerror(return_code));
    return return_string;
}
