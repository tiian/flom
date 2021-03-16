/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_HANDLE_H
# define FLOM_HANDLE_H



#include <stdlib.h>



#include "flom_errors.h"
#include "flom_types.h"



/**
 * This scalar type is used to represent the state of an handle
 */
typedef enum flom_handle_state_e {
    /**
     * Initial state
     */
    FLOM_HANDLE_STATE_INIT = 22,
    /**
     * The client is connected to the daemon and the resource is NOT locked
     */
    FLOM_HANDLE_STATE_CONNECTED,
    /**
     * The client is connected to the daemon and the resource is locked
     */
    FLOM_HANDLE_STATE_LOCKED,
    /**
     * The client is NOT connected to the daemon
     */
    FLOM_HANDLE_STATE_DISCONNECTED,
    /**
     * The handle memory was released and the handle itself can NOT be used
     * without a call to @ref flom_handle_init method
     */
    FLOM_HANDLE_STATE_CLEANED
} flom_handle_state_t;



/**
 * This object is used to save all the necessary context to interact with
 * libflom library.
 * Some fields use "void *" type to avoid useless internal details exposure
 * (flom methods proxies the correct types)
 */
typedef struct flom_handle_s {
    /**
     * Handle state
     */
    flom_handle_state_t   state;
    /**
     * Connection data
     */
    void                 *conn;
    /**
     * Configuration data
     */
    void                 *config;
    /**
     * (last) Locked element (useful for resource sets)
     */
    char                 *locked_element;
} flom_handle_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initializes an object handle; this function MUST be called before the
     * first usage of a new statically allocated handle or after an handle
     * has been cleaned up with function @ref flom_handle_clean
     * @param handle (Input/Output): a statically allocated object to
     * initialize
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_init(flom_handle_t *handle);
    


    /**
     * Cleans an object handle; this function MUST be called before the handle
     * will go out of scope of an handle;
     * if this method is not called a memory leak will be generated.
     * For every object initialized with @ref flom_handle_init
     * there must be a call to this method.
     * If the handle is locked, it will be unlocked before proceeding with
     * clean-up to avoid memory leaks.
     * @param handle (Input/Output): a statically allocated object to clean
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_clean(flom_handle_t *handle);
    


    /**
     * Allocates and initializes (using function @ref flom_handle_init) a
     * new dynamically allocated object handle
     * @return a new object handle or NULL if any error happens
     */
    flom_handle_t *flom_handle_new(void);


    
    /**
     * Cleans (using function @ref flom_handle_clean) and deallocates an
     * object allocated (created) with function @ref flom_handle_new
     * @param handle (Input): a dynamically allocated object to delete
     */
    void flom_handle_delete(flom_handle_t *handle);


    
    /**
     * Locks the (logical) resource linked to an handle; the resource MUST
     * be unlocked using function @ref flom_handle_unlock when the lock
     * condition is no more necessary
     * @param handle (Input/Output): a valid object handle
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_lock(flom_handle_t *handle);



    /**
     * Unlocks the (logical) resource linked to an handle; the resource MUST
     * be previously locked using function @ref flom_handle_lock
     * @param handle (Input/Output): a valid object handle
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_unlock(flom_handle_t *handle);



    /**
     * Unlocks the (logical) resource linked to an handle and rollback the
     * transactional resource state; the resource MUST be previously locked
     * using function @ref flom_handle_lock . This method should be used only
     * with transactional resources, for example: transactional unique
     * sequences
     * @param handle (Input/Output): a valid object handle
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_unlock_rollback(flom_handle_t *handle);



    /**
     * Return the name of the locked element if the resource is of type set.<P>
     * Note 1: this function can be used only after @ref flom_handle_lock
     *         and before @ref flom_handle_unlock<P>
     * Note 2: this function can be used only when locking a resource of
     *         type "resource set"<P>
     * Note 3: the return string must copied as soon as possible to a different
     *         place because it's a dynamic string removed by
     *         @ref flom_handle_unlock<P>
     * @param handle (Input): a valid object handle
     * @return the name of the locked element
     */
    const char *flom_handle_get_locked_element(const flom_handle_t *handle) {
        return handle->locked_element;
    }


    
    /**
     * Get the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_get_multicast_address,
     *         @ref flom_handle_get_multicast_port).
     * The current value can be altered using function
     *         @ref flom_handle_set_discovery_attempts
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_discovery_attempts(const flom_handle_t *handle);

    
    
    /**
     * Set the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_set_multicast_address,
     *         @ref flom_handle_set_multicast_port).
     * The current value can be inspected using function
     *         @ref flom_handle_get_discovery_attempts
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_discovery_attempts(flom_handle_t *handle, int value);



    /**
     * Get the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_get_multicast_address,
     *         @ref flom_handle_get_multicast_port).
     * The current value can be altered using function
     *         @ref flom_handle_set_discovery_timeout
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_discovery_timeout(const flom_handle_t *handle);

    
    
    /**
     * Set the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_set_multicast_address,
     *         @ref flom_handle_set_multicast_port).
     * The current value can be inspected using function
     *         @ref flom_handle_get_discovery_timeout.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_discovery_timeout(flom_handle_t *handle, int value);



    /**
     * Get the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be altered using function
     *         @ref flom_handle_set_discovery_ttl
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_discovery_ttl(const flom_handle_t *handle);

    
    
    /**
     * Set the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html 
     * . The current value can be inspected using function
     *         @ref flom_handle_get_discovery_ttl.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_discovery_ttl(flom_handle_t *handle, int value);



    /**
     * Get lock mode property: how a simple or hierarchical resource will
     * be locked when function @ref flom_handle_lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be altered using function
     *         @ref flom_handle_set_lock_mode
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    flom_lock_mode_t flom_handle_get_lock_mode(const flom_handle_t *handle);



    /**
     * Set lock mode property: how a simple or hierarchical resource will
     * be locked when function @ref flom_handle_lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be inspected using function
     *         @ref flom_handle_get_lock_mode
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_lock_mode(flom_handle_t *handle,
                                  flom_lock_mode_t value);



    /**
     * Get the multicast address: the IP address (or a network name that the
     * system can resolve) of the IP multicast group that must be contacted
     * to reach FLoM daemon (server) using UDP/IP; see also
     *         @ref flom_handle_get_multicast_port.
     * The current value can be altered using function
     *         @ref flom_handle_set_multicast_address.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_multicast_address(const flom_handle_t *handle);



    /**
     * Set the multicast address: the IP address (or a network name that the
     * system can resolve) of the IP multicast group that must be contacted
     * to reach FLoM daemon (server) using UDP/IP; see also
     *         @ref flom_handle_set_multicast_port.
     * The current value can be inspected using function
     *         @ref flom_handle_get_multicast_address.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_multicast_address(flom_handle_t *handle,
                                          const char *value);


    
    /**
     * Get the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also
     *         @ref flom_handle_get_multicast_address.
     * The current value can be altered using function
     *         @ref flom_handle_set_multicast_port.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_multicast_port(const flom_handle_t *handle);



    /**
     * Set the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also
     *         @ref flom_handle_set_multicast_address.
     * The current value can be inspected using function
     *         @ref flom_handle_get_multicast_port.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_multicast_port(flom_handle_t *handle, int value);



    /**
     * Get the network interface that must be used for IPv6 link local
     * addresses
     * The current value can be altered using function
     *         @ref flom_handle_set_network_interface.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_network_interface(const flom_handle_t *handle);



    /**
     * Set the network interface that must be used for IPv6 link local
     * addresses
     * The current value can be inspected using function
     *         @ref flom_handle_get_network_interface.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return a reason code
     */
    int flom_handle_set_network_interface(flom_handle_t *handle,
                                          const char *value);


    
    /**
     * Get "resource create" boolean property: it specifies if function
     * @ref flom_handle_lock can create a new resource when the specified
     * one is not defined; the default value is TRUE. 
     * The current value can be altered using function
     *     @ref flom_handle_set_resource_create.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_resource_create(const flom_handle_t *handle);


    
    /**
     * Set "resource create" boolean property: it specifies if function
     * @ref flom_handle_lock can create a new resource when the specified
     * one is not defined.
     * The current value can be inspected using function
     *         @ref flom_handle_get_resource_create.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_resource_create(flom_handle_t *handle, int value);



    /**
     * Get "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released it;
     * the expiration is necessary to avoid useless resource allocation.
     * The current value can be altered using function
     *     @ref flom_handle_set_resource_idle_lifespan.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_resource_idle_lifespan(const flom_handle_t *handle);


    
    /**
     * Set "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released it;
     * the expiration is necessary to avoid useless resource allocation.
     * The current value can be inspected using function
     *     @ref flom_handle_get_resource_idle_lifespan.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_resource_idle_lifespan(flom_handle_t *handle,
                                               int value);



    /**
     * Get the resource name: the name of the resource that can be locked and
     * unlocked using @ref flom_handle_lock and @ref flom_handle_unlock
     * functions.
     * The current value can be altered using function
     *     @ref flom_handle_set_resource_name.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_resource_name(const flom_handle_t *handle);


    
    /**
     * Set the resource name: the name of the resource that can be locked and
     * unlocked using @ref flom_handle_lock and @ref flom_handle_unlock
     * functions.
     * The current value can be inspected using function
     *     @ref flom_handle_get_resource_name.
     * NOTE: the resource type is determined by its name; take a look to
     * flom command man page (-r, --resource-name option) for an
     * explanation of the resource name grammar.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_resource_name(flom_handle_t *handle,
                                      const char *value);


    
    /**
     * Get "resource quantity" property: the number of units that will be
     * locked and unlocked using @ref flom_handle_lock and
     * @ref flom_handle_unlock functions.
     * The current value can be altered using function
     *     @ref flom_handle_set_resource_quantity.
     * NOTE: this property applies to "numeric resources" only.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_resource_quantity(const flom_handle_t *handle);

    

    /**
     * Set "resource quantity" property: the number of units that will be
     * locked and unlocked using @ref flom_handle_lock and
     * @ref flom_handle_unlock functions.
     * The current value can be inspected using function
     *     @ref flom_handle_get_resource_quantity.
     * NOTE: this property applies to "numeric resources" only.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_resource_quantity(flom_handle_t *handle, int value);


    
    /**
     * Get "resource timeout" property: how long a lock operation
     * (see @ref flom_handle_lock) will wait if the resource is locked
     * by another requester.
     * The current value can be altered using function
     *     @ref flom_handle_set_resource_timeout.
     * @param handle (Input): a valid object handle
     * @return the current value: <BR>
     *        0: no wait <BR>
     *        >0: maximum number of milliseconds to wait <BR>
     *        <0: unlimited wait
     */
    int flom_handle_get_resource_timeout(const flom_handle_t *handle);


    
    /**
     * Set "resource timeout" property: how long a lock operation
     * (see @ref flom_handle_lock) will wait if the resource is locked
     * by another requester.
     * The current value can be inspected using function
     *     @ref flom_handle_get_resource_timeout.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value: <BR>
     *        0: no wait <BR>
     *        >0: maximum number of milliseconds to wait <BR>
     *        <0: unlimited wait
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_resource_timeout(flom_handle_t *handle, int value);



    /**
     * Get the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be altered using function
     *         @ref flom_handle_set_socket_name.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_socket_name(const flom_handle_t *handle);

    

    /**
     * Set the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be inspected using function
     *         @ref flom_handle_get_socket_name.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_set_socket_name(flom_handle_t *handle,
                                    const char *value);


    
    /**
     * Get the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be altered using function
     *     @ref flom_handle_set_trace_filename.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_trace_filename(const flom_handle_t *handle);


    
    /**
     * Set the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be inspected using function
     *     @ref flom_handle_get_trace_filename.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK
     */
    int flom_handle_set_trace_filename(flom_handle_t *handle,
                                       const char *value);



    /**
     * Get the unicast address: the IP address (or a network name that the
     * system can resolve) of the host that must be contacted
     * to reach FLoM daemon (server) using TCP/IP; see also
     *         @ref flom_handle_get_unicast_port.
     * The current value can be altered using function
     *         @ref flom_handle_set_unicast_address.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_unicast_address(const flom_handle_t *handle);



    /**
     * Set the unicast address: the IP address (or a network name that the
     * system can resolve) of the host that must be contacted
     * to reach FLoM daemon (server) using TCP/IP; see also
     *         @ref flom_handle_set_unicast_port.
     * The current value can be inspected using function
     *         @ref flom_handle_get_unicast_address.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_unicast_address(flom_handle_t *handle,
                                        const char *value);


    
    /**
     * Get the TCP/IP unicast port that must be used to contact the FLoM
     * daemon (server) using TCP/IP; see also
     *         @ref flom_handle_get_unicast_address.
     * The current value can be altered using function
     *         @ref flom_handle_set_unicast_port.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_unicast_port(const flom_handle_t *handle);


    
    /**
     * Set the TCP/IP unicast port that must be used to contact the FLoM
     * daemon (server) using TCP/IP; see also
     *         @ref flom_handle_set_unicast_address.
     * The current value can be inspected using function
     *         @ref flom_handle_get_unicast_port.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_unicast_port(flom_handle_t *handle, int value);



    /**
     * Get the TLS certificate file name.
     * The current value can be altered using function
     *         @ref flom_handle_set_tls_certificate.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_tls_certificate(const flom_handle_t *handle);



    /**
     * Set the TLS certificate file name.
     * The current value can be inspected using function
     *         @ref flom_handle_get_tls_certificate.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_tls_certificate(flom_handle_t *handle,
                                        const char *value);


    
    /**
     * Get the TLS private key file name.
     * The current value can be altered using function
     *         @ref flom_handle_set_tls_private_key.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_tls_private_key(const flom_handle_t *handle);



    /**
     * Set the TLS private key file name.
     * The current value can be inspected using function
     *         @ref flom_handle_get_tls_private_key.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_tls_private_key(flom_handle_t *handle,
                                        const char *value);


    
    /**
     * Get the TLS private key file name.
     * The current value can be altered using function
     *         @ref flom_handle_set_tls_ca_certificate.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    const char *flom_handle_get_tls_ca_certificate(
        const flom_handle_t *handle);



    /**
     * Set the TLS private key file name.
     * The current value can be inspected using function
     *         @ref flom_handle_get_tls_ca_certificate.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_tls_ca_certificate(flom_handle_t *handle,
                                           const char *value);


    
    /**
     * Get the TLS check peer id boolean flag
     * The current value can be altered using function
     *         @ref flom_handle_set_tls_check_peer_id.
     * @param handle (Input): a valid object handle
     * @return the current (boolean) value
     */
    int flom_handle_get_tls_check_peer_id(const flom_handle_t *handle);


    
    /**
     * Set the TLS check peer id boolean flag
     * The current value can be inspected using function
     *         @ref flom_handle_get_tls_check_peer_id.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new (boolean) value
     * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
     */
    int flom_handle_set_tls_check_peer_id(flom_handle_t *handle, int value);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_HANDLE_H */
