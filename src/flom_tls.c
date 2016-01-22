/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_STRING_H
# include <string.h>
#endif



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



const char *flom_tls_get_error_label(int error)
{
    switch (error) {
        case SSL_ERROR_NONE:
            return "SSL_ERROR_NONE";
        case SSL_ERROR_SSL:
            return "SSL_ERROR_SSL";
        case SSL_ERROR_WANT_READ:
            return "SSL_ERROR_WANT_READ";
        case SSL_ERROR_WANT_WRITE:
            return "SSL_ERROR_WANT_WRITE";
        case SSL_ERROR_WANT_X509_LOOKUP:
            return "SSL_ERROR_WANT_X509_LOOKUP";
        case SSL_ERROR_SYSCALL:
            return "SSL_ERROR_SYSCALL";
        case SSL_ERROR_ZERO_RETURN:
            return "SSL_ERROR_ZERO_RETURN";
        case SSL_ERROR_WANT_CONNECT:
            return "SSL_ERROR_WANT_CONNECT";
        case SSL_ERROR_WANT_ACCEPT:
            return "SSL_ERROR_WANT_ACCEPT";
        default:
            return "__NOT_KNOWN_CODE__";
    } /* switch (error) */
}



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

    /* reset object content */
    memset(obj, 0, sizeof(flom_tls_t));
    /* initialize a valid context */
    obj->client = client;
    obj->depth = FLOM_TLS_MAX_DEPTH_CERT_CHAIN_VERIF;
    return;
}



void flom_tls_free(flom_tls_t *obj) {
    if (NULL != obj->ssl) {
        SSL_free(obj->ssl);
        obj->ssl = NULL;
    }
    if (NULL != obj->ctx) {
        SSL_CTX_free(obj->ctx);
        obj->ctx = NULL;
    }
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
        int mode;
        
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
        
        /* set callback data index */
        obj->callback_data_index = SSL_get_ex_new_index(
            0, "callback data index", NULL, NULL, NULL);
        /* set callback function */
        mode = SSL_VERIFY_PEER;
        if (!obj->client)
            mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
        FLOM_TRACE(("flom_tls_context: SSL_CTX_set_verify(%p, %d, "
                    "flom_tls_callback)\n", obj->ctx, mode));
        SSL_CTX_set_verify(obj->ctx, mode, flom_tls_callback);
        /* set max depth for the certificate chain verification */
        SSL_CTX_set_verify_depth(obj->ctx, obj->depth+1);
        
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



int flom_tls_callback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    int ret_cod = TRUE;
    FLOM_TRACE(("flom_tls_callback: preverify_ok=%d\n", preverify_ok));
    FLOM_TRACE(("flom_tls_callback: ret_cod=%d\n", ret_cod));
    return ret_cod;
}



int flom_tls_set_cert(flom_tls_t *obj, const char *cert_file,
                      const char *priv_key_file, const char *ca_cert_file)
{
    enum Exception { SSL_CTX_USE_CERTIFICATE_FILE_ERROR
                     , SSL_CTX_USE_PRIVATEKEY_FILE_ERROR
                     , SSL_CTX_CHECK_PRIVATE_KEY_ERROR
                     , SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_set_cert\n"));
    TRY {
        /* loads the first certificate stored in file into ctx */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_use_certificate_file("
                    "obj->ctx, '%s', SSL_FILETYPE_PEM)\n", cert_file));
        if (1 != SSL_CTX_use_certificate_file(
                obj->ctx, cert_file, SSL_FILETYPE_PEM)) {
            FLOM_TRACE_SSLERR(("flom_tls_set_cert/"
                               "SSL_CTX_use_certificate_file:"));
            THROW(SSL_CTX_USE_CERTIFICATE_FILE_ERROR);
        }
        /* adds the first private key found in file to ctx */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_use_PrivateKey_file("
                    "obj->ctx, '%s', SSL_FILETYPE_PEM)\n", priv_key_file));
        if (1 != SSL_CTX_use_PrivateKey_file(
                obj->ctx, priv_key_file, SSL_FILETYPE_PEM)) {
            FLOM_TRACE_SSLERR(("flom_tls_set_cert/"
                               "SSL_CTX_use_PrivateKey_file:"));
            THROW(SSL_CTX_USE_PRIVATEKEY_FILE_ERROR);
        }
        
        /* checks the consistency of a private key with the corresponding
           certificate loaded into ctx */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_check_private_key("
                    "obj->ctx)\n"));
        if (1 != SSL_CTX_check_private_key(obj->ctx)) {
            FLOM_TRACE_SSLERR(("flom_tls_set_cert/"
                               "SSL_CTX_check_private_key:"));
            THROW(SSL_CTX_CHECK_PRIVATE_KEY_ERROR);
        }

        /* specifies the locations for ctx, at which CA certificates for
           verification purposes are located */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_load_verify_locations("
                    "obj->ctx, '%s', NULL)\n", ca_cert_file));
        if (1 != SSL_CTX_load_verify_locations(
                obj->ctx, ca_cert_file, NULL)) {
            FLOM_TRACE_SSLERR(("flom_tls_set_cert:"));
            THROW(SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SSL_CTX_USE_CERTIFICATE_FILE_ERROR:
                ret_cod = FLOM_RC_SSL_CTX_USE_CERTIFICATE_FILE_ERROR;
                break;
            case SSL_CTX_USE_PRIVATEKEY_FILE_ERROR:
                ret_cod = FLOM_RC_SSL_CTX_USE_PRIVATEKEY_FILE_ERROR;
                break;
            case SSL_CTX_CHECK_PRIVATE_KEY_ERROR:
                ret_cod = FLOM_RC_SSL_CTX_CHECK_PRIVATE_KEY_ERROR;
                break;
            case SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR:
                ret_cod = FLOM_RC_SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_set_cert/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tls_prepare(flom_tls_t *obj, int sockfd)
{
    enum Exception { SSL_NEW_ERROR
                     , SSL_SET_FD_ERROR
                     , SSL_SET_EX_DATA_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_prepare\n"));
    TRY {
        /* SSL struct must be NULL */
        if (NULL != obj->ssl) {
            FLOM_TRACE(("flom_tls_prepare: ssl struct is not NULL (%p), "
                        "freeing it before allocating a new one!\n"));
            SSL_free(obj->ssl);
            obj->ssl = NULL;
        }
        /* creates a new SSL structure which is needed to hold the data for
           a TLS/SSL connection */
        if (NULL == (obj->ssl = SSL_new(obj->ctx))) {
            FLOM_TRACE_SSLERR(("flom_tls_prepare/SSL_new:"));
            THROW(SSL_NEW_ERROR);
        }
        /* sets the file descriptor sockfd as the input/output facility for
           the TLS/SSL */
        if (1 != SSL_set_fd(obj->ssl, sockfd)) {
            FLOM_TRACE_SSLERR(("flom_tls_prepare/SSL_set_fd:"));
            THROW(SSL_SET_FD_ERROR);
        }
        /* store application data at arg for idx into the ssl object */
        if (1 != SSL_set_ex_data(obj->ssl, obj->callback_data_index,
                                 &obj->callback_data)) {
            FLOM_TRACE_SSLERR(("flom_tls_prepare/SSL_set_ex_data:"));
            THROW(SSL_SET_EX_DATA_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SSL_NEW_ERROR:
                ret_cod = FLOM_RC_SSL_NEW_ERROR;
                break;
            case SSL_SET_FD_ERROR:
                ret_cod = FLOM_RC_SSL_SET_FD_ERROR;
                break;
            case SSL_SET_EX_DATA_ERROR:
                ret_cod = FLOM_RC_SSL_SET_EX_DATA_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_prepare/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tls_connect(flom_tls_t *obj, int sockfd)
{
    enum Exception { TSL_PREPARE_ERROR
                     , SSL_CONNECT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_connect\n"));
    TRY {
        int rc;
        
        /* SSL boilerplate... */
        if (FLOM_RC_OK != (ret_cod = flom_tls_prepare(obj, sockfd)))
            THROW(TSL_PREPARE_ERROR);
        /* initiates the TLS/SSL handshake with the server */
        if (SSL_ERROR_NONE != (rc = SSL_get_error(
                                   obj->ssl, SSL_connect(obj->ssl)))) {
            FLOM_TRACE(("flom_tls_connect/SSL_connect: SSL error=%d (%s)\n",
                        rc, flom_tls_get_error_label(rc)));
            FLOM_TRACE_SSLERR(("flom_tls_connect/SSL_connect:"));
            THROW(SSL_CONNECT_ERROR);
        } else {
            FLOM_TRACE(("flom_tls_connect: connection established with %s "
                        "encryption\n",
                        SSL_CIPHER_get_name(SSL_get_current_cipher(
                                                obj->ssl))));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TSL_PREPARE_ERROR:
                break;
            case SSL_CONNECT_ERROR:
                ret_cod = FLOM_RC_SSL_CONNECT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_connect/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tls_accept(flom_tls_t *obj, int sockfd)
{
    enum Exception { TSL_PREPARE_ERROR
                     , SSL_ACCEPT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_accept\n"));
    TRY {
        int rc;
        
        /* SSL boilerplate... */
        if (FLOM_RC_OK != (ret_cod = flom_tls_prepare(obj, sockfd)))
            THROW(TSL_PREPARE_ERROR);
        /* initiates the TLS/SSL handshake with the server */
        if (SSL_ERROR_NONE != (rc = SSL_get_error(
                                   obj->ssl, SSL_accept(obj->ssl)))) {
            FLOM_TRACE(("flom_tls_accept/SSL_accept: SSL error=%d (%s)\n",
                        rc, flom_tls_get_error_label(rc)));
            FLOM_TRACE_SSLERR(("flom_tls_accept/SSL_accept:"));
            THROW(SSL_ACCEPT_ERROR);
        } else {
            FLOM_TRACE(("flom_tls_accepted: connection accepted with %s "
                        "encryption\n",
                        SSL_CIPHER_get_name(SSL_get_current_cipher(
                                                obj->ssl))));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TSL_PREPARE_ERROR:
                break;
            case SSL_ACCEPT_ERROR:
                ret_cod = FLOM_RC_SSL_ACCEPT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_accept/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

