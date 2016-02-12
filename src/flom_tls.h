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
 * Maximum depth for the certificate chain verification
 */
#define FLOM_TLS_MAX_DEPTH_CERT_CHAIN_VERIF    3



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
 * Breakdown structure used to store all the strings related to a certificate
 */
struct flom_tls_cert_s {
    /**
     * Country Name string
     */
    gchar *c_str;
    /**
     * State or Province Name string
     */
    gchar *st_str;
    /**
     * Organization Name string
     */
    gchar *o_str;
    /**
     * Organizational Unit Name NID
     */
    gchar *ou_str;
    /**
     * Common Name string
     */
    gchar *cn_str;
};



/**
 * Object to store the metadata strings that can be retrieved by a "standard"
 * X509 certificate for issuer and subject
 */
typedef struct {
    struct flom_tls_cert_s issuer;
    struct flom_tls_cert_s subject;
} flom_tls_cert_t;



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
     * SSL structure
     */
    SSL                                  *ssl;
    /**
     * Boolean: is the TLS connection related to the client side (TRUE) or
     *          the server side (FALSE)
     */
    int                                   client;
    /**
     * Maximum depth for the certificate chain verification
     */
    int                                   depth;
    /**
     * Certificate returned by the peer
     */
    flom_tls_cert_t                      *cert;
    /**
     * Struct used to pass custom data to callback function
     */
    struct flom_tls_callback_data_s       callback_data;
    /**
     * Index to custom callback data
     */
    int                                   callback_data_index;
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
     * Numeric/string conversion for some SSL errors...
     * Why is not supplied by the library?!?! Where am I wrong?!?!
     * @return a human readable label
     */
    const char *flom_tls_get_error_label(int error);
    
    

    /**
     * Create a new TLS/SSL object and, 
     * initialize (OpenSSL) TLS/SSL library if necessary
     * @param client IN the TLS connection is related to the client side
     *        (TRUE) or to the server side (FALSE)
     */
    flom_tls_t *flom_tls_new(int client);

    

    /**
     * Release the TLS/SSL object created and encapsulated by flom_tls_t;
     * NOTE: the object must be initialized before this method can be applied
     * @param obj IN/OUT TLS object
     */
    void flom_tls_delete(flom_tls_t *obj);


    
    /**
     * Create a new TLS/SSL context
     * @param obj IN/OUT connection object reference
     * @return a reason code
     */
    int flom_tls_context(flom_tls_t *obj);



    /**
     * Verify callback function
     * @param preverify_ok IN indicates, whether the verification of the
     *        certificate in question was passed (preverify_ok=1) or not
     *        (preverify_ok=0)
     * @param x509_ctx IN is a pointer to the complete context used for the
     *        certificate chain verification
     * @return a boolean value
     */
    int flom_tls_callback(int preverify_ok, X509_STORE_CTX *x509_ctx);



    /**
     * Set local certificate and private key from files
     * @param obj IN/OUT connection object reference
     * @param cert_file IN local certificate filename
     * @param priv_key_file IN private key filename
     * @param ca_cert_file IN ca certificate filename
     * @return a reason code
     */
    int flom_tls_set_cert(flom_tls_t *obj, const char *cert_file,
                          const char *priv_key_file, const char *ca_cert_file);



    /**
     * Prepare a standard TCP connection to be switched to a TLS connection
     * @param obj IN/OUT TLS object
     * @param sockfd IN file descriptor of the socket already connected
     * @return a reason code
     */
    int flom_tls_prepare(flom_tls_t *obj, int sockfd);
    

    
    /**
     * Switch a standard TCP client connection to a TLS client connection
     * @param obj IN/OUT TLS object
     * @param sockfd IN file descriptor of the socket already connected
     * @return a reason code
     */
    int flom_tls_connect(flom_tls_t *obj, int sockfd);
    

    
    /**
     * Switch a standard TCP server connection to a TLS server connection
     * @param obj IN/OUT TLS object
     * @param sockfd IN file descriptor of the socket already connected
     * @return a reason code
     */
    int flom_tls_accept(flom_tls_t *obj, int sockfd);
    


    /**
     * Retrieve the certificate presented by the peer
     * @param obj IN/OUT TLS object
     * @return a reason code
     */
    int flom_tls_check_peer_cert(flom_tls_t *obj);



    /**
     * Create a new object of type flom_tls_cert_t
     * @return a new object or NULL if any error occurred
     */
    static inline flom_tls_cert_t *flom_tls_cert_new() {
        return (flom_tls_cert_t *)g_try_malloc0(sizeof(flom_tls_cert_t));
    }



    /**
     * Fills in all the strings related to the struct
     * @param s IN/OUT struct names
     * @param nid IN field identificator
     * @param str IN value associated to the field
     * @return a reason code
     */
    int flom_tls_cert_struct_fill(struct flom_tls_cert_s *s, int nid,
                                  const unsigned char *str);


    
    /**
     * Release all the strings related to the struct
     * @param s IN struct names
     */
    void flom_tls_cert_struct_delete(struct flom_tls_cert_s *s);

    
    
    /**
     * Delete an object previously created with @ref flom_tls_cert_new
     * @param obj IN TLS object
     */
    void flom_tls_cert_delete(flom_tls_cert_t *obj);



    /**
     * Parse the X509 certificate passed by the peer and extract all the
     * "standard" metada for "issuer" and "subject"
     * @param obj IN/OUT TLS object
     * @param ssl IN SSL object (as in OpenSSL library)
     * @return a reason code
     */
    int flom_tls_cert_parse(flom_tls_cert_t *obj, SSL *ssl);


    
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
