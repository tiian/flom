/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>


#include "flom_config.h"
#include "flom_errors.h"
#include "flom_tls.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_TLS



int flom_tls_initialized = FALSE;

/**
 * This mutex is used to double initialization of the OpenSSL library
 */
GStaticMutex flom_tls_mutex = G_STATIC_MUTEX_INIT;



void flom_tls_init(flom_tls_t *obj, int client)
{
    /* initialize OpenSSL library if necessary */
    /* lock the mutex */
    g_static_mutex_lock(&flom_tls_mutex);
    if (!flom_tls_initialized) {
        FLOM_TRACE(("flom_tls_init: calling SSL_load_error_strings()...\n"));
        SSL_load_error_strings();
        FLOM_TRACE(("flom_tls_init: calling SSL_library_init()...\n"));
        SSL_library_init();
        flom_tls_initialized = TRUE;
    }
    /* remove the lock from the mutex */
    g_static_mutex_unlock(&flom_tls_mutex);

    /* initialize a valid context */
    obj->client = client;
    return;
}



int flom_tls_context(flom_tls_t *obj)
{
    enum Exception { TLS_NO_VALID_METHOD
                     , SSL_CTX_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_context\n"));
    TRY {
        SSL_METHOD *method = NULL;
        const char *side = obj->client ? "client" : "server";
        /* set TLS/SSL method */
#ifdef HAVE_TLS_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLS_%s_method()\n", side));
        method = obj->client ? TLS_client_method() : TLS_server_method();
#elif HAVE_TLSV1_2_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_2_%s_method()\n", side));
        method = obj->client ? TLSv1_2_client_method() :
            TLSv1_2_server_method();
#elif HAVE_TLSV1_1_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_1_%s_method()\n", side));
        method = obj->client ? TLSv1_1_client_method() :
            TLSv1_1_server_method();
#elif HAVE_TLSV1_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_%s_method()\n", side));
        method = obj->client ? TLSv1_client_method() : TLSv1_server_method();
#elif HAVE_SSLV3_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "SSLv3_%s_method()\n", side));
        method = obj->client ? SSLv3_client_method() : SSLv3_server_method();
#endif
        if (NULL == method) {
            FLOM_TRACE(("flom_tls_context: no valid method is "
                        "available\n"));
            THROW(TLS_NO_VALID_METHOD)
        }

        /* create a mew context from method */
        if (NULL == (obj->ctx = SSL_CTX_new(method))) {
            FLOM_TRACE_SSLERR(("flom_tls_context/SSL_CTX_new:"));
            THROW(SSL_CTX_NEW_ERROR);
        }
        
        /* set custom data struct */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_NO_VALID_METHOD:
                ret_cod = FLOM_RC_TLS_NO_VALID_METHOD;
                break;
            case SSL_CTX_NEW_ERROR:
                ret_cod = FLOM_RC_SSL_CTX_NEW_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_context/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

