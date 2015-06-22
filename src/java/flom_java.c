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
JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_newFH(
    JNIEnv *env, jobject this_obj)
{
    enum Exception { MALLOC_ERROR
                     , GET_OBJECT_CLASS_ERROR
                     , GET_FIELD_ID_ERROR
                     , NEW_DIRECT_BYTE_BUFFER_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    jobject byte_buffer = NULL;

    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFH\n"));
    TRY {
        jclass this_class;
        jfieldID field_id;
        jobject byte_buffer;
        flom_handle_t *h = NULL;

        /* create a new native handle */
        if (NULL == (h = flom_handle_new()))
            THROW(MALLOC_ERROR);
        /* get a reference to this object's class */
        if (NULL == (this_class = (*env)->GetObjectClass(env, this_obj))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFH: "
                        "this_class == NULL\n"));
            THROW(GET_OBJECT_CLASS_ERROR);
        }
        /* get the field identificator */
        if (NULL == (field_id = (*env)->GetFieldID(
                         env, this_class, "NativeHandler",
                         "Ljava/nio/ByteBuffer;"))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFH: "
                        "field_id == NULL\n"));
            THROW(GET_FIELD_ID_ERROR);
        }
        /* create ByteBuffer */
        if (NULL == (byte_buffer = (*env)->NewDirectByteBuffer(
                         env, (void *)h, sizeof(flom_handle_t))))
            THROW(NEW_DIRECT_BYTE_BUFFER_ERROR);
        /* set ByteBuffer reference */
        (*env)->SetObjectField(env, this_obj, field_id, byte_buffer);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MALLOC_ERROR:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case GET_OBJECT_CLASS_ERROR:
                ret_cod = FLOM_RC_GET_OBJECT_CLASS_ERROR;
                break;
            case GET_FIELD_ID_ERROR:
                ret_cod = FLOM_RC_GET_FIELD_ID_ERROR;
                break;
            case NEW_DIRECT_BYTE_BUFFER_ERROR:
                ret_cod = FLOM_RC_NEW_DIRECT_BYTE_BUFFER_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newFH/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



/* This is an helper internal function, it's not seen by JNI */
int Java_org_tiian_flom_FlomHandle_getNativeHandle(
    JNIEnv *env, jobject this_obj, flom_handle_t **handle)
{
    enum Exception { GET_OBJECT_CLASS_ERROR
                     , GET_FIELD_ID_ERROR
                     , NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle\n"));
    TRY {
        jclass this_class;
        jfieldID field_id;
        jobject byte_buffer;
        flom_handle_t *fh;
        
        /* get a reference to this object's class */
        if (NULL == (this_class = (*env)->GetObjectClass(env, this_obj))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle: "
                        "this_class == NULL\n"));
            THROW(GET_OBJECT_CLASS_ERROR);
        }
        /* get the field identificator */
        if (NULL == (field_id = (*env)->GetFieldID(
                         env, this_class, "NativeHandler",
                         "Ljava/nio/ByteBuffer;"))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle: "
                        "field_id == NULL\n"));
            THROW(GET_FIELD_ID_ERROR);
        }
        /* get ByteBuffer reference */
        byte_buffer = (*env)->GetObjectField(env, this_obj, field_id);
        /* cast to flom_handle_t */
        if (NULL == (fh = (flom_handle_t *)(*env)->GetDirectBufferAddress(
                         env, byte_buffer)))
            THROW(NULL_OBJECT);
        *handle = fh;
        
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
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_deleteFH
  (JNIEnv *env, jobject this_obj)
{
    enum Exception { GET_NATIVE_HANDLE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;

    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFH\n"));
    TRY {
        flom_handle_t *handle;
        
        if (FLOM_RC_OK != (
                ret_cod = Java_org_tiian_flom_FlomHandle_getNativeHandle(
                    env, this_obj, &handle)))
            THROW(GET_NATIVE_HANDLE_ERROR);
        flom_handle_delete(handle);
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GET_NATIVE_HANDLE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteFH/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return (jint)ret_cod;
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_lockFH
(JNIEnv *env, jobject this_obj)
{
    enum Exception { GET_NATIVE_HANDLE_ERROR
                     , LOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_lockFH\n"));
    TRY {
        flom_handle_t *handle;
        
        if (FLOM_RC_OK != (
                ret_cod = Java_org_tiian_flom_FlomHandle_getNativeHandle(
                    env, this_obj, &handle)))
            THROW(GET_NATIVE_HANDLE_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_handle_lock(handle)))
            THROW(LOCK_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GET_NATIVE_HANDLE_ERROR:
            case LOCK_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_lockFH/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_unlockFH
(JNIEnv *env, jobject this_obj)
{
    enum Exception { GET_NATIVE_HANDLE_ERROR
                     , UNLOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_unlockFH\n"));
    TRY {
        flom_handle_t *handle;
        
        if (FLOM_RC_OK != (
                ret_cod = Java_org_tiian_flom_FlomHandle_getNativeHandle(
                    env, this_obj, &handle)))
            THROW(GET_NATIVE_HANDLE_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(handle)))
            THROW(UNLOCK_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GET_NATIVE_HANDLE_ERROR:
            case UNLOCK_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_unlockFH/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getLockedElementFH
(JNIEnv *env, jobject this_obj)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("\n"));
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
    FLOM_TRACE(("/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
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
