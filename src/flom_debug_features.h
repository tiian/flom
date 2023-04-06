/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef DEBUG_FEATURES_H
# define DEBUG_FEATURES_H



#include <config.h>



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DEBUG_FEATURES



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


    /**
     * Label associated to IPv6 Multicast Server debug feature
     */
    extern const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_SERVER;
    /**
     * Label associated to IPv6 Multicast Client debug feature
     */
    extern const char *FLOM_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT;
    /**
     * Label associated to TLS Server debug feature
     */
    extern const char *FLOM_DEBUG_FEATURES_TLS_SERVER;
    /**
     * Label associated to TLS Client debug feature
     */
    extern const char *FLOM_DEBUG_FEATURES_TLS_CLIENT;



    /**
     * Execute a debug feature
     * @param name IN of the feature
     * @return a reason code
     */
    int flom_debug_features(const char *name);

    

    /**
     * Execute an IPv6 Multicast Server to test network connectivity
     * @return a reason code
     */
    int flom_debug_features_ipv6_multicast_server(void);



    /**
     * Execute an IPv6 Multicast Client to test network connectivity
     * @return a reason code
     */
    int flom_debug_features_ipv6_multicast_client(void);
    


    /**
     * Execute a TLS Server to test SSL/TLS security setup
     * @return a reason code
     */
    int flom_debug_features_tls_server(void);
    


    /**
     * Execute a TLS Client to test SSL/TLS security setup
     * @return a reason code
     */
    int flom_debug_features_tls_client(void);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* DEBUG_FEATURES_H */
