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
#ifndef FLOM_TLS_H
# define FLOM_TLS_H



#include <config.h>


#ifdef HAVE_OPENSSL_SSL_H
# include <openssl/ssl.h>
#endif
#ifdef HAVE_OPENSSL_ERR_H
# include <openssl/err.h>
#endif
#ifdef HAVE_OPENSSL_X509V3_H
# include <openssl/x509v3.h>
#endif



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_TLS



/**
 * Custom data passed to verify callback OpenSSL function
 */
struct flom_tls_callback_data_s {
    /**
     * Certification chain verification depth
     */
    int      depth;
    /**
     * Don't stop, do another step
     */
    int      dont_stop;
};



/**
 * Object used to manage a TLS connection: it contains all the necessary data
 * and avoid the usage of static data
 */
typedef struct {
    /**
     * SSL context
     */
    SSL_CTX                              *ctx;
    /**
     * Boolean: is the TLS connection related to the client side (TRUE) or
     *          the server side (FALSE)
     */
    int                                   client;
    /**
     * Struct used to pass custom data to callback function
     */
    struct flom_tls_callback_data_s       callback_data;
} flom_tls_t;



/**
 * Status of OpenSSL library initialization: TRUE = initialized,
 * FALSE = not initialized
 */
extern int flom_tls_initialized;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize (OpenSSL) TLS/SSL library and an object that must be used
     * for the next operations
     * @param obj OUT the object that must be initialized
     * @param client IN the TLS connection is related to the client side
     *        (TRUE) or to the server side (FALSE)
     */
    void flom_tls_init(flom_tls_t *obj, int client);

    

    /**
     * Create a new TLS/SSL context
     * @param obj IN/OUT the connection object
     * @return a reason code
     */
    int flom_tls_context(flom_tls_t *obj);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_TLS_H */
