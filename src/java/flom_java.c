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
#include "config.h"



#include <jni.h>
#include <stdio.h>
#include "flom_defines.h"
#include "flom_trace.h"
#include "flom.h"
#include "flom_java_FlomHandle.h"
#include "flom_java_FlomErrorCodes.h"
 


/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_API



/*
 * This is not an API and is not available in flom_handle.h; this definition
 * has the only purpose to avoid a warning message
 */
int flom_init_check(void);



/* Allocate a new FlomHandle (native struct) */
JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_newJNI(
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
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newJNI\n"));
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
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newJNI: "
                        "this_class == NULL\n"));
            THROW(GET_OBJECT_CLASS_ERROR);
        }
        /* get the field identificator */
        if (NULL == (field_id = (*env)->GetFieldID(
                         env, this_class, "NativeHandler",
                         "Ljava/nio/ByteBuffer;"))) {
            FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newJNI: "
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
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_newJNI/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



/* This is an helper internal function, it's not seen by JNI */
flom_handle_t *Java_org_tiian_flom_FlomHandle_getNativeHandle(
    JNIEnv *env, jobject this_obj)
{
    jclass this_class;
    jfieldID field_id;
    jobject byte_buffer;
    flom_handle_t *fh;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle\n"));

    /* get a reference to this object's class */
    if (NULL == (this_class = (*env)->GetObjectClass(env, this_obj))) {
        FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle: "
                    "this_class == NULL\n"));
        jclass Exception = (*env)->FindClass(env, "java/lang/Exception");
        (*env)->ThrowNew(
            env, Exception,
            "JNI/Java_org_tiian_flom_FlomHandle_getNativeHandle/"
            "GetObjectClass returned NULL");
    }

    /* get the field identificator */
    if (NULL == (field_id = (*env)->GetFieldID(
                     env, this_class, "NativeHandler",
                     "Ljava/nio/ByteBuffer;"))) {
        FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle: "
                    "field_id == NULL\n"));
        jclass Exception = (*env)->FindClass(env, "java/lang/Exception");
        (*env)->ThrowNew(
            env, Exception,
            "JNI/Java_org_tiian_flom_FlomHandle_getNativeHandle/"
            "GetFieldID returned NULL");
    }
    /* get ByteBuffer reference */
    byte_buffer = (*env)->GetObjectField(env, this_obj, field_id);
    /* cast to flom_handle_t */
    if (NULL == (fh = (flom_handle_t *)(*env)->GetDirectBufferAddress(
                     env, byte_buffer))) {
        FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNativeHandle: "
                    "fh == NULL\n"));
        jclass Exception = (*env)->FindClass(env, "java/lang/Exception");
        (*env)->ThrowNew(
            env, Exception,
            "JNI/Java_org_tiian_flom_FlomHandle_getNativeHandle/"
            "GetDirectBufferAddress returned NULL");
    }
    return fh;
}



JNIEXPORT void JNICALL Java_org_tiian_flom_FlomHandle_deleteJNI
  (JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_deleteJNI\n"));
    flom_handle_delete(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_lockJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_lockJNI\n"));
    return flom_handle_lock(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_unlockJNI
(JNIEnv *env, jobject this_obj)
{
    enum Exception { UNLOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_unlockJNI\n"));
    return flom_handle_unlock(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_unlockRollbackJNI
(JNIEnv *env, jobject this_obj)
{
    enum Exception { UNLOCK_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    if (FLOM_RC_OK != (ret_cod = flom_init_check()))
        return ret_cod;
    
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_unlockRollbackJNI\n"));
    return flom_handle_unlock_rollback(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj));
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getLockedElementJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getLockedElementJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_locked_element(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getDiscoveryAttemptsJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getDiscoveryAttemptsJNI\n"));
    return (jint)flom_handle_get_discovery_attempts(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setDiscoveryAttemptsJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setDiscoveryAttemptsJNI\n"));
    return (jint)flom_handle_set_discovery_attempts(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        value);
}

    

JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getDiscoveryTimeoutJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getDiscoveryTimeoutJNI\n"));
    return (jint)flom_handle_get_discovery_timeout(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setDiscoveryTimeoutJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setDiscoveryTimeoutJNI\n"));
    return (jint)flom_handle_set_discovery_timeout(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}

    

JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getDiscoveryTtlJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getDiscoveryTtlJNI\n"));
    return (jint)flom_handle_get_discovery_ttl(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setDiscoveryTtlJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setDiscoveryTtlJNI\n"));
    return (jint)flom_handle_set_discovery_ttl(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}

    

JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getLockModeJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getLockModeJNI\n"));
    return (jint)flom_handle_get_lock_mode(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setLockModeJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setLockModeJNI\n"));
    return (jint)flom_handle_set_lock_mode(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}

    

JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getMulticastAddressJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getMulticastAddressJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_multicast_address(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setMulticastAddressJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setMulticastAddressJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_multicast_address(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}


    
JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getMulticastPortJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getMulticastPortJNI\n"));
    return (jint)flom_handle_get_multicast_port(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setMulticastPortJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setMulticastPortJNI\n"));
    return (jint)flom_handle_set_multicast_port(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getNetworkInterfaceJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getNetworkInterfaceJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_network_interface(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setNetworkInterfaceJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setNetworkInterfaceJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_network_interface(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj), cstr);
}


    
JNIEXPORT jboolean JNICALL Java_org_tiian_flom_FlomHandle_getResourceCreateJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getResourceCreateJNI\n"));
    return (jint)flom_handle_get_resource_create(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setResourceCreateJNI
  (JNIEnv *env, jobject this_obj, jboolean value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setResourceCreateJNI\n"));
    return (jint)flom_handle_set_resource_create(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getResourceIdleLifespanJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getResourceIdleLifespanJNI\n"));
    return (jint)flom_handle_get_resource_idle_lifespan(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setResourceIdleLifespanJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setResourceIdleLifespanJNI\n"));
    return (jint)flom_handle_set_resource_idle_lifespan(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getResourceNameJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getResourceNameJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_resource_name(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setResourceNameJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setResourceNameJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_resource_name(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getResourceQuantityJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getResourceQuantityJNI\n"));
    return (jint)flom_handle_get_resource_quantity(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setResourceQuantityJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setResourceQuantityJNI\n"));
    return (jint)flom_handle_set_resource_quantity(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getResourceTimeoutJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getResourceTimeoutJNI\n"));
    return (jint)flom_handle_get_resource_timeout(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setResourceTimeoutJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setResourceTimeoutJNI\n"));
    return (jint)flom_handle_set_resource_timeout(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getSocketNameJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getSocketNameJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_socket_name(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setSocketNameJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setSocketNameJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_socket_name(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}


    
JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getTraceFilenameJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getTraceFilenameJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_trace_filename(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setTraceFilenameJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setTraceFilenameJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_trace_filename(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getUnicastAddressJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getUnicastAddressJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_unicast_address(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setUnicastAddressJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setUnicastAddressJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_unicast_address(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_getUnicastPortJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getUnicastPortJNI\n"));
    return (jint)flom_handle_get_unicast_port(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setUnicastPortJNI
(JNIEnv *env, jobject this_obj, jint value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setUnicastPortJNI\n"));
    return (jint)flom_handle_set_unicast_port(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getTlsCertificateJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getTlsCertificateJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_tls_certificate(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setTlsCertificateJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setTlsCertificateJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_tls_certificate(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getTlsPrivateKeyJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getTlsPrivateKeyJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_tls_private_key(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setTlsPrivateKeyJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setTlsPrivateKeyJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_tls_private_key(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomHandle_getTlsCaCertificateJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getTlsCaCertificateJNI\n"));
    return (*env)->NewStringUTF(
        env,
        flom_handle_get_tls_ca_certificate(
            Java_org_tiian_flom_FlomHandle_getNativeHandle(
                env, this_obj)));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setTlsCaCertificateJNI
(JNIEnv *env, jobject this_obj, jstring value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setTlsCaCertificateJNI\n"));
    const char *cstr = (*env)->GetStringUTFChars(env, value, NULL);
    return (jint)flom_handle_set_tls_ca_certificate(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(env, this_obj),
        cstr);
}



JNIEXPORT jboolean JNICALL Java_org_tiian_flom_FlomHandle_getTlsCheckPeerIdJNI
(JNIEnv *env, jobject this_obj)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_getTlsCheckPeerIdJNI\n"));
    return (jint)flom_handle_get_tls_check_peer_id(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj));
}



JNIEXPORT jint JNICALL Java_org_tiian_flom_FlomHandle_setTlsCheckPeerIdJNI
  (JNIEnv *env, jobject this_obj, jboolean value)
{
    FLOM_TRACE(("Java_org_tiian_flom_FlomHandle_setTlsCheckPeerIdJNI\n"));
    return (jint)flom_handle_set_tls_check_peer_id(
        Java_org_tiian_flom_FlomHandle_getNativeHandle(
            env, this_obj), value);
}



JNIEXPORT jstring JNICALL Java_org_tiian_flom_FlomErrorCodes_getText
(JNIEnv *env, jclass this_obj, jint code)
{
    return (*env)->NewStringUTF(env, flom_strerror(code));
}
