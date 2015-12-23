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



void flom_tls_init(void)
{
    FLOM_TRACE(("flom_tls_init: calling SSL_load_error_strings()...\n"));
    SSL_load_error_strings();
    FLOM_TRACE(("flom_tls_init: calling SSL_library_init()...\n"));
    SSL_library_init();
    return;
}



int flom_tls_create_context(SSL_CTX **ctx, int client)
{
    enum Exception { TLS_NO_VALID_METHOD
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_create_context\n"));
    TRY {
        SSL_METHOD *method = NULL;
#ifdef HAVE_TLS_METHOD
        FLOM_TRACE(("flom_tls_create_context: setting TLS/SSL method to "
                    "TLS_*_method\n"));
        method = client ? TLS_client_method() : TLS_server_method();
#elif HAVE_TLSV1_2_METHOD
        FLOM_TRACE(("flom_tls_create_context: setting TLS/SSL method to "
                    "TLSv1_2_*_method\n"));
        method = client ? TLSv1_2_client_method() : TLSv1_2_server_method();
#elif HAVE_TLSV1_1_METHOD
        FLOM_TRACE(("flom_tls_create_context: setting TLS/SSL method to "
                    "TLSv1_1_*_method\n"));
        method = client ? TLSv1_1_client_method() : TLSv1_1_server_method();
#elif HAVE_TLSV1_METHOD
        FLOM_TRACE(("flom_tls_create_context: setting TLS/SSL method to "
                    "TLSv1_*_method\n"));
        method = client ? TLSv1_client_method() : TLSv1_server_method();
#elif HAVE_SSLV3_METHOD
        FLOM_TRACE(("flom_tls_create_context: setting TLS/SSL method to "
                    "SSLv3_*_method\n"));
        method = client ? SSLv3_client_method() : SSLv3_server_method();
#endif
        if (NULL == method) {
            FLOM_TRACE(("flom_tls_create_context: no valid method is "
                        "available\n"));
            THROW(TLS_NO_VALID_METHOD)
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_NO_VALID_METHOD:
                ret_cod = FLOM_RC_TLS_NO_VALID_METHOD;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_create_context/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

