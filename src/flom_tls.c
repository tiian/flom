/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
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
#include "config.h"



#ifdef HAVE_STRING_H
# include <string.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_syslog.h"
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



gchar *flom_tls_get_unique_id() {
    gchar *tmp = NULL;
    char *machine_id = dbus_get_local_machine_id();
    tmp = g_strdup(machine_id);
    dbus_free(machine_id);
    return tmp;
}



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



flom_tls_t *flom_tls_new(int client)
{
    flom_tls_t *tmp;
    
    /* initialize OpenSSL library if necessary */
    /* lock the mutex */
    g_static_mutex_lock(&flom_tls_mutex);
    if (!flom_tls_initialized) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        FLOM_TRACE(("flom_tls_init: calling SSL_library_init()...\n"));
        SSL_library_init();
#else
        FLOM_TRACE(("flom_tls_init: calling OPENSSL_init_ssl()...\n"));
        OPENSSL_init_ssl(0, NULL);
#endif
        FLOM_TRACE(("flom_tls_init: calling SSL_load_error_strings()...\n"));
        SSL_load_error_strings();
        FLOM_TRACE(("flom_tls_init: calling "
                    "OpenSSL_add_all_algorithms()...\n"));
        OpenSSL_add_all_algorithms();
        flom_tls_initialized = TRUE;
    }
    /* remove the lock from the mutex */
    g_static_mutex_unlock(&flom_tls_mutex);

    /* allocate a new object */
    tmp = g_try_malloc0(sizeof(flom_tls_t));

    /* initialize a valid context */
    if (NULL != tmp) {
        tmp->client = client;
        tmp->depth = FLOM_TLS_MAX_DEPTH_CERT_CHAIN_VERIF;
    }
    return tmp;
}



void flom_tls_delete(flom_tls_t *obj) {
    /* check the object is not NULL... */
    if (NULL == obj)
        return;
    if (NULL != obj->cert) {
        flom_tls_cert_delete(obj->cert);
        obj->cert = NULL;
    }
    if (NULL != obj->ssl) {
        SSL_free(obj->ssl);
        obj->ssl = NULL;
    }
    if (NULL != obj->ctx) {
        SSL_CTX_free(obj->ctx);
        obj->ctx = NULL;
    }
    /* remove the object itself */
    g_free(obj);
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

        /*
         * set TLS method:
         * if TLS_method is available, this is the preferred choice
         * else the lowest TLSv1_method will be used to be compatible with
         * OpenSSL 0.9.8k
         */
#ifdef HAVE_TLS_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLS_%s_method()\n", side));
        method = obj->client ? TLS_client_method() : TLS_server_method();
#elif HAVE_TLSV1_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_%s_method()\n", side));
        method = obj->client ? TLSv1_client_method() : TLSv1_server_method();
#elif HAVE_TLSV1_1_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_1_%s_method()\n", side));
        method = obj->client ? TLSv1_1_client_method() :
            TLSv1_1_server_method();
#elif HAVE_TLSV1_2_METHOD
        FLOM_TRACE(("flom_tls_context: setting TLS/SSL method to "
                    "TLSv1_2_%s_method()\n", side));
        method = obj->client ? TLSv1_2_client_method() :
            TLSv1_2_server_method();
#endif
        if (NULL == method) {
            FLOM_TRACE(("flom_tls_context: no valid method is "
                        "available\n"));
            THROW(TLS_NO_VALID_METHOD)
        }

        /* create a mew context from method */
        if (NULL == (obj->ctx = SSL_CTX_new(method))) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_context/SSL_CTX_new:", err);
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
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_set_cert/"
                              "SSL_CTX_use_certificate_file:", err);
            THROW(SSL_CTX_USE_CERTIFICATE_FILE_ERROR);
        }
        /* adds the first private key found in file to ctx */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_use_PrivateKey_file("
                    "obj->ctx, '%s', SSL_FILETYPE_PEM)\n", priv_key_file));
        if (1 != SSL_CTX_use_PrivateKey_file(
                obj->ctx, priv_key_file, SSL_FILETYPE_PEM)) {
            unsigned long err = ERR_get_error();
            char buf[1024];
            FLOM_TRACE_SSLERR("flom_tls_set_cert/"
                              "SSL_CTX_use_PrivateKey_file:", err);
            ERR_error_string_n(err, buf, sizeof(buf));
            syslog(LOG_ERR, FLOM_SYSLOG_FLM018E, buf);
            THROW(SSL_CTX_USE_PRIVATEKEY_FILE_ERROR);
        }
        
        /* checks the consistency of a private key with the corresponding
           certificate loaded into ctx */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_check_private_key("
                    "obj->ctx)\n"));
        if (1 != SSL_CTX_check_private_key(obj->ctx)) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_set_cert/"
                              "SSL_CTX_check_private_key:", err);
            THROW(SSL_CTX_CHECK_PRIVATE_KEY_ERROR);
        }

        /* specifies the locations for ctx, at which CA certificates for
           verification purposes are located */
        FLOM_TRACE(("flom_tls_set_cert: SSL_CTX_load_verify_locations("
                    "obj->ctx, '%s', NULL)\n", ca_cert_file));
        if (1 != SSL_CTX_load_verify_locations(
                obj->ctx, ca_cert_file, NULL)) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_set_cert:", err);
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
                        "freeing it before allocating a new one!\n",
                        obj->ssl));
            SSL_free(obj->ssl);
            obj->ssl = NULL;
        }
        /* creates a new SSL structure which is needed to hold the data for
           a TLS/SSL connection */
        if (NULL == (obj->ssl = SSL_new(obj->ctx))) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_prepare/SSL_new:", err);
            THROW(SSL_NEW_ERROR);
        }
        /* sets the file descriptor sockfd as the input/output facility for
           the TLS/SSL */
        if (1 != SSL_set_fd(obj->ssl, sockfd)) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_prepare/SSL_set_fd:", err);
            THROW(SSL_SET_FD_ERROR);
        }
        /* store application data at arg for idx into the ssl object */
        if (1 != SSL_set_ex_data(obj->ssl, obj->callback_data_index,
                                 &obj->callback_data)) {
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_prepare/SSL_set_ex_data:", err);
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
                     , TLS_CERT_PARSE_ERROR
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
            unsigned long err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_connect/SSL_connect:", err);
            THROW(SSL_CONNECT_ERROR);
        } else {
            FLOM_TRACE(("flom_tls_connect: connection established with %s "
                        "encryption\n",
                        SSL_CIPHER_get_name(SSL_get_current_cipher(
                                                obj->ssl))));
        }
        /* get peer certificate */
        obj->cert = flom_tls_cert_new();
        if (FLOM_RC_OK != (ret_cod = flom_tls_cert_parse(
                               obj->cert, obj->ssl)))
            THROW(TLS_CERT_PARSE_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TSL_PREPARE_ERROR:
                break;
            case SSL_CONNECT_ERROR:
                ret_cod = FLOM_RC_SSL_CONNECT_ERROR;
                break;
            case TLS_CERT_PARSE_ERROR:
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
                     , TLS_CERT_PARSE_ERROR
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
            unsigned long err;
            FLOM_TRACE(("flom_tls_accept/SSL_accept: SSL error=%d (%s)\n",
                        rc, flom_tls_get_error_label(rc)));
            err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_accept/SSL_accept:", err);
            THROW(SSL_ACCEPT_ERROR);
        } else {
            FLOM_TRACE(("flom_tls_accepted: connection accepted with %s "
                        "encryption\n",
                        SSL_CIPHER_get_name(SSL_get_current_cipher(
                                                obj->ssl))));
        }
        /* get peer certificate */
        obj->cert = flom_tls_cert_new();
        if (FLOM_RC_OK != (ret_cod = flom_tls_cert_parse(
                               obj->cert, obj->ssl)))
            THROW(TLS_CERT_PARSE_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TSL_PREPARE_ERROR:
                break;
            case SSL_ACCEPT_ERROR:
                ret_cod = FLOM_RC_SSL_ACCEPT_ERROR;
                break;
            case TLS_CERT_PARSE_ERROR:
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



int flom_tls_send(flom_tls_t *obj, const void *buf, size_t len)
{
    enum Exception { SSL_WRITE_ERROR
                     , SEND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_send\n"));
    TRY {
        int sent_bytes, ssl_rc;

        sent_bytes = SSL_write(obj->ssl, buf, (int)len);
        if (0 >= sent_bytes) {
            unsigned long err;
            ssl_rc = SSL_get_error(obj->ssl, sent_bytes);
            FLOM_TRACE(("flom_tls_send/SSL_write: SSL error=%d (%s)\n",
                        ssl_rc, flom_tls_get_error_label(ssl_rc)));
            err = ERR_get_error();
            FLOM_TRACE_SSLERR("flom_tls_send/SSL_write:", err);
            THROW(SSL_WRITE_ERROR);
        } else if (sent_bytes != (int)len) {
            FLOM_TRACE(("flom_tls_send: send %d instead of %d bytes\n",
                        sent_bytes, (int)len));
            THROW(SEND_ERROR);
        } /* if (0 >= sent_bytes) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SSL_WRITE_ERROR:
                ret_cod = FLOM_RC_SSL_WRITE_ERROR;
                break;
            case SEND_ERROR:
                ret_cod = FLOM_RC_SEND_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_send/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tls_recv_msg(flom_tls_t *obj, char *buf, size_t len,
                      size_t *received)
{
    enum Exception {
        OUT_OF_RANGE,
        SSL_READ_ERROR,
        BUFFER_OVERFLOW,
        NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_recv_msg\n"));
    TRY {
        char closing_tag[FLOM_MSG_BUFFER_SIZE];
        size_t retrieved = 0;
        size_t closing_tag_len;
        size_t to_be_read;
        int read_bytes;
        char closing_tag_last;
        int found = FALSE;
        int i, j;
        
        /* preparing the closing tag string */
        snprintf(closing_tag, sizeof(closing_tag), "</%s>",
                 FLOM_MSG_TAG_MSG);
        closing_tag_len = strlen(closing_tag);
        closing_tag_last = closing_tag[closing_tag_len-1];
        FLOM_TRACE(("flom_tls_recv_msg: closing_tag='%s', closing_tag_len="
                    SIZE_T_FORMAT ", closing_tag_last='%c'\n",
                    closing_tag, closing_tag_len, closing_tag_last));
        if (len < closing_tag_len)
            THROW(OUT_OF_RANGE);
        /* loop until a complete message has been retrieved or an error
           occurs */
        to_be_read = closing_tag_len;
        while (!found) {
            read_bytes = SSL_read(obj->ssl, buf+retrieved, (int)to_be_read);
            FLOM_TRACE(("flom_tls_recv_msg: read_bytes=%d '%*.*s'\n",
                        read_bytes, read_bytes, read_bytes, buf+retrieved));
            if (0 >= read_bytes) {
                unsigned long err;
                int ssl_rc = SSL_get_error(obj->ssl, read_bytes);
                FLOM_TRACE(("flom_tls_recv_msg/SSL_read: SSL error=%d (%s)\n",
                            ssl_rc, flom_tls_get_error_label(ssl_rc)));
                err = ERR_get_error();
                FLOM_TRACE_SSLERR("flom_tls_recv_msg/SSL_read:", err);
                THROW(SSL_READ_ERROR);
            }
            retrieved += read_bytes;
            /* too few chars, go on */
            if (retrieved < closing_tag_len)
                continue;
            /* looping on closing_tag */
            j = 0;
            for (i=0; i<closing_tag_len; ++i) {
                if (buf[retrieved-j-1] == closing_tag[closing_tag_len-i-1])
                    j++;
                else
                    j = 0;
            } /* for (i=0; i<closing_tag_len; ++i) */
            if (j == closing_tag_len)
                found = TRUE;
            else {
                to_be_read = closing_tag_len - j;
                if (retrieved + to_be_read >= len)
                    THROW(BUFFER_OVERFLOW);
            }
        } /* while (TRUE) */
        /* put string terminator */
        buf[retrieved] = '\0';
        *received = retrieved;
        FLOM_TRACE(("flom_tls_recv_msg: received message is '%s' "
                    "of " SIZE_T_FORMAT " chars\n", buf, *received));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case SSL_READ_ERROR:
                ret_cod = FLOM_RC_SSL_READ_ERROR;
                break;
            case BUFFER_OVERFLOW:
                ret_cod = FLOM_RC_BUFFER_OVERFLOW;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_recv_msg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_tls_cert_struct_delete(struct flom_tls_cert_s *s) {
    if (NULL != s->c_str) {
        g_free(s->c_str);
        s->c_str = NULL;
    }
    if (NULL != s->st_str) {
        g_free(s->st_str);
        s->st_str = NULL;
    }
    if (NULL != s->l_str) {
        g_free(s->l_str);
        s->l_str = NULL;
    }
    if (NULL != s->o_str) {
        g_free(s->o_str);
        s->o_str = NULL;
    }
    if (NULL != s->ou_str) {
        g_free(s->ou_str);
        s->ou_str = NULL;
    }
    if (NULL != s->e_str) {
        g_free(s->e_str);
        s->e_str = NULL;
    }
    if (NULL != s->cn_str) {
        g_free(s->cn_str);
        s->cn_str = NULL;
    }
}



int flom_tls_cert_struct_fill(struct flom_tls_cert_s *s, int nid,
                              const unsigned char *str)
{
    enum Exception { G_STRDUP_ERROR1
                     , G_STRDUP_ERROR2
                     , G_STRDUP_ERROR3
                     , G_STRDUP_ERROR4
                     , G_STRDUP_ERROR5
                     , G_STRDUP_ERROR6
                     , G_STRDUP_ERROR7
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_cert_struct_fill\n"));
    TRY {
        switch (nid) {
            case NID_countryName:
                if (NULL == (s->c_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR1);
                break;
            case NID_stateOrProvinceName:
                if (NULL == (s->st_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR2);
                break;
            case NID_localityName:
                if (NULL == (s->l_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR3);
                break;
            case NID_organizationName:
                if (NULL == (s->o_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR4);
                break;
            case NID_organizationalUnitName:
                if (NULL == (s->ou_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR5);
                break;
            case NID_pkcs9_emailAddress:
                if (NULL == (s->e_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR6);
                break;
            case NID_commonName:
                if (NULL == (s->cn_str = g_strdup((const gchar *)str)))
                    THROW(G_STRDUP_ERROR7);
                break;
            default:
                FLOM_TRACE(("flom_tls_cert_struct_fill: NID=%d (%s/%s) "
                            "ignored\n", nid, OBJ_nid2sn(nid),
                            OBJ_nid2ln(nid)));
        } /* switch (nid) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR1:
            case G_STRDUP_ERROR2:
            case G_STRDUP_ERROR3:
            case G_STRDUP_ERROR4:
            case G_STRDUP_ERROR5:
            case G_STRDUP_ERROR6:
            case G_STRDUP_ERROR7:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_cert_struct_fill/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_tls_cert_delete(flom_tls_cert_t *obj) {
    if (NULL == obj)
        return;
    flom_tls_cert_struct_delete(&obj->issuer);
    flom_tls_cert_struct_delete(&obj->subject);
    g_free(obj);
}



int flom_tls_cert_parse(flom_tls_cert_t *obj, SSL *ssl)
{
    enum Exception { NO_CERTIFICATE
                     , TLS_CERT_STRUCT_FILL_ERROR1
                     , TLS_CERT_STRUCT_FILL_ERROR2
                     , SSL_GET_VERIFY_RESULT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    X509 *cert = NULL;
    
    FLOM_TRACE(("flom_tls_cert_parse\n"));
    TRY {
        X509_NAME *issuer_name, *subject_name;
        int i;
        long verify_result;
        
        /* retrieve certificate */
        if (NULL == (cert = SSL_get_peer_certificate(ssl))) {
            FLOM_TRACE(("flom_tls_cert_parse: there's no available peer "
                        "certificate\n"));
            syslog(LOG_WARNING, FLOM_SYSLOG_FLM010W);
            THROW(NO_CERTIFICATE);
        }

        /* retrieve issuer and subject names */
        issuer_name = X509_get_issuer_name(cert);
        subject_name = X509_get_subject_name(cert);

        /* retrieving certificate fields for issuer */
        for (i=0; i<X509_NAME_entry_count(issuer_name); ++i) {
            X509_NAME_ENTRY *e = X509_NAME_get_entry(issuer_name, i);
            ASN1_STRING *d = X509_NAME_ENTRY_get_data(e);
            unsigned char *str = ASN1_STRING_data(d);
            ASN1_OBJECT *o = X509_NAME_ENTRY_get_object(e);
            int nid = OBJ_obj2nid(o);
            if (FLOM_RC_OK != (ret_cod = flom_tls_cert_struct_fill(
                                   &obj->issuer, nid, str)))
                THROW(TLS_CERT_STRUCT_FILL_ERROR1);
        } /* for (i=0; i<X509_NAME_entry_count(issuer_name); ++i) */
        FLOM_TRACE(("flom_tls_cert_parse: issuer fields are "
                    "%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s\n",
                    OBJ_nid2sn(NID_countryName), STRORNULL(obj->issuer.c_str),
                    OBJ_nid2sn(NID_stateOrProvinceName),
                    STRORNULL(obj->issuer.st_str),
                    OBJ_nid2sn(NID_localityName),
                    STRORNULL(obj->issuer.l_str),
                    OBJ_nid2sn(NID_organizationName),
                    STRORNULL(obj->issuer.o_str),
                    OBJ_nid2sn(NID_organizationalUnitName),
                    STRORNULL(obj->issuer.ou_str),
                    OBJ_nid2sn(NID_pkcs9_emailAddress),
                    STRORNULL(obj->issuer.e_str),
                    OBJ_nid2sn(NID_commonName),
                    STRORNULL(obj->issuer.cn_str)));
        syslog(LOG_INFO, FLOM_SYSLOG_FLM011I,
               OBJ_nid2sn(NID_countryName), STRORNULL(obj->issuer.c_str),
               OBJ_nid2sn(NID_stateOrProvinceName),
               STRORNULL(obj->issuer.st_str),
               OBJ_nid2sn(NID_localityName),
               STRORNULL(obj->issuer.l_str),
               OBJ_nid2sn(NID_organizationName),
               STRORNULL(obj->issuer.o_str),
               OBJ_nid2sn(NID_organizationalUnitName),
               STRORNULL(obj->issuer.ou_str),
               OBJ_nid2sn(NID_pkcs9_emailAddress),
               STRORNULL(obj->issuer.e_str),
               OBJ_nid2sn(NID_commonName),
               STRORNULL(obj->issuer.cn_str));
        
        /* retrieving certificate fields for subject */
        for (i=0; i<X509_NAME_entry_count(subject_name); ++i) {
            X509_NAME_ENTRY *e = X509_NAME_get_entry(subject_name, i);
            ASN1_STRING *d = X509_NAME_ENTRY_get_data(e);
            unsigned char *str = ASN1_STRING_data(d);
            ASN1_OBJECT *o = X509_NAME_ENTRY_get_object(e);
            int nid = OBJ_obj2nid(o);
            if (FLOM_RC_OK != (ret_cod = flom_tls_cert_struct_fill(
                                   &obj->subject, nid, str)))
                THROW(TLS_CERT_STRUCT_FILL_ERROR2);
        } /* for (i=0; i<X509_NAME_entry_count(subject_name); ++i) */
        FLOM_TRACE(("flom_tls_cert_parse: subject fields are "
                    "%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s\n",
                    OBJ_nid2sn(NID_countryName), STRORNULL(obj->subject.c_str),
                    OBJ_nid2sn(NID_stateOrProvinceName),
                    STRORNULL(obj->subject.st_str),
                    OBJ_nid2sn(NID_localityName),
                    STRORNULL(obj->subject.l_str),
                    OBJ_nid2sn(NID_organizationName),
                    STRORNULL(obj->subject.o_str),
                    OBJ_nid2sn(NID_organizationalUnitName),
                    STRORNULL(obj->subject.ou_str),
                    OBJ_nid2sn(NID_pkcs9_emailAddress),
                    STRORNULL(obj->subject.e_str),
                    OBJ_nid2sn(NID_commonName),
                    STRORNULL(obj->subject.cn_str)));
        syslog(LOG_INFO, FLOM_SYSLOG_FLM012I,
               OBJ_nid2sn(NID_countryName), STRORNULL(obj->subject.c_str),
               OBJ_nid2sn(NID_stateOrProvinceName),
               STRORNULL(obj->subject.st_str),
               OBJ_nid2sn(NID_localityName),
               STRORNULL(obj->subject.l_str),
               OBJ_nid2sn(NID_organizationName),
               STRORNULL(obj->subject.o_str),
               OBJ_nid2sn(NID_organizationalUnitName),
               STRORNULL(obj->subject.ou_str),
               OBJ_nid2sn(NID_pkcs9_emailAddress),
               STRORNULL(obj->subject.e_str),
               OBJ_nid2sn(NID_commonName),
               STRORNULL(obj->subject.cn_str));
        
        /* verify TLS/SSL chain */
        if (X509_V_OK != (verify_result = SSL_get_verify_result(ssl))) {
            FLOM_TRACE(("flom_tls_cert_parse/SSL_get_verify_result: "
                        "verify_result=%ld\n", verify_result));
            syslog(LOG_ERR, FLOM_SYSLOG_FLM017E, verify_result);
            THROW(SSL_GET_VERIFY_RESULT_ERROR);
        } /* if (X509_V_OK != (verify_result... */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NO_CERTIFICATE:
                ret_cod = FLOM_RC_NO_CERTIFICATE;
                break;
            case TLS_CERT_STRUCT_FILL_ERROR1:
            case TLS_CERT_STRUCT_FILL_ERROR2:
                break;
            case SSL_GET_VERIFY_RESULT_ERROR:
                ret_cod = FLOM_RC_SSL_GET_VERIFY_RESULT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* memory clean-up */
    if (NULL != cert) {
        X509_free(cert);
        cert = NULL;
    }
    
    FLOM_TRACE(("flom_tls_cert_parse/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_tls_cert_check(flom_tls_t *obj, const gchar *peer_unique_id,
                        const gchar *peer_address)
{
    enum Exception { NULL_OBJECT
                     ,UNIQUE_ID_DOES_NOT_MATCH
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_tls_cert_check\n"));
    TRY {
        FLOM_TRACE(("flom_tls_cert_check: peer address='%s', CN='%s', peer "
                    "unique ID='%s'\n",
                    STRORNULL(peer_address),
                    STRORNULL(obj->cert->subject.cn_str),
                    STRORNULL(peer_unique_id)));
        if (NULL == obj->cert->subject.cn_str || NULL == peer_unique_id)
            THROW(NULL_OBJECT);
        if (0 != strcmp(obj->cert->subject.cn_str, peer_unique_id)) {
            FLOM_TRACE(("flom_tls_cert_check: the unique ID provided by the "
                        "peer does not match the CN field inside the "
                        "presented X509 certificate\n"));
            syslog(LOG_ERR, FLOM_SYSLOG_FLM013E, peer_address, peer_unique_id,
                   obj->cert->subject.cn_str);
            THROW(UNIQUE_ID_DOES_NOT_MATCH);
        }
        syslog(LOG_INFO, FLOM_SYSLOG_FLM014I, peer_address, peer_unique_id,
               obj->cert->subject.cn_str);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case UNIQUE_ID_DOES_NOT_MATCH:
                ret_cod = FLOM_RC_UNIQUE_ID_DOES_NOT_MATCH;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_tls_cert_check/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

