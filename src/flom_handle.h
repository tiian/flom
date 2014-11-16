/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_HANDLE_H
# define FLOM_HANDLE_H



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
    void                 *conn_data;
    /**
     * Configuration data
     */
    void                 *config;
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
     * @param element (Output): contains the name of the locked element if
     *        the resource is of type "resource set"; set it to NULL if you
     *        are not interested to retrieve an element name
     * @param element_size (Input): maximum number of characters (null
     *        terminator included) that can be used by the function to store
     *        the name of the locked element
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_lock(flom_handle_t *handle,
                         char *element, size_t element_size);



    /**
     * Unlocks the (logical) resource linked to an handle; the resource MUST
     * be previously locked using function @ref flom_handle_lock
     * @param handle (Input/Output): a valid object handle
     * @return a reason code (see file @ref flom_errors.h)
     */
    int flom_handle_unlock(flom_handle_t *handle);



    /**
     * Gets the maximum number of attempts that will be tryed during
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
     * Sets the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_set_multicast_address,
     *         @ref flom_handle_set_multicast_port).
     * The current value can be inspected using function
     *         @ref flom_handle_get_discovery_attempts
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_discovery_attempts(flom_handle_t *handle, int value);



    /**
     * Gets the number of milliseconds between two consecutive attempts that
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
     * Sets the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     *         @ref flom_handle_set_multicast_address,
     *         @ref flom_handle_set_multicast_port).
     * The current value can be inspected using function
     *         @ref flom_handle_get_discovery_timeout.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_discovery_timeout(flom_handle_t *handle, int value);



    /**
     * Gets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be altered using function
     *         @ref flom_handle_set_discovery_ttl
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_discovery_ttl(const flom_handle_t *handle);

    
    
    /**
     * Sets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html 
     * . The current value can be inspected using function
     *         @ref flom_handle_get_discovery_ttl.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_discovery_ttl(flom_handle_t *handle, int value);



    /**
     * Gets lock mode property: how a simple or hierarchical resource will
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
     * Sets lock mode property: how a simple or hierarchical resource will
     * be locked when function @ref flom_handle_lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be inspected using function
     *         @ref flom_handle_get_lock_mode
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_lock_mode(flom_handle_t *handle,
                                   flom_lock_mode_t value);



    /**
     * Gets multicast address: the IP address (or a network name that the
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
     * Sets multicast address: the IP address (or a network name that the
     * system can resolve) of the IP multicast group that must be contacted
     * to reach FLoM daemon (server) using UDP/IP; see also
     *         @ref flom_handle_set_multicast_port.
     * The current value can be inspected using function
     *         @ref flom_handle_get_multicast_address.
     * @param handle (Input/Output): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_multicast_address(flom_handle_t *handle,
                                           const char *value);


    
    /**
     * Gets UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also
     *         @ref flom_handle_get_multicast_address.
     * The current value can be altered using function
     *         @ref flom_handle_set_multicast_port.
     * @param handle (Input): a valid object handle
     * @return the current value
     */
    int flom_handle_get_multicast_port(const flom_handle_t *handle);



    /**
     * Sets UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also
     *         @ref flom_handle_set_multicast_address.
     * The current value can be inspected using function
     *         @ref flom_handle_get_multicast_port.
     * @param handle (Input): a valid object handle
     * @param value (Input): the new value
     */
    void flom_handle_set_multicast_port(flom_handle_t *handle, int value);


    /* @@@ restart from here; above function documentation already revised */

    /**
     * Set UNIX (AF_LOCAL) socket name for client/server communication
     * @param handle IN/OUT library handle
     * @param value IN the name that must be used for the socket
     * @return a reason code
     */
    int flom_handle_set_socket_name(flom_handle_t *handle,
                                    const char *value);


    
    /**
     * Get UNIX (AF_LOCAL) socket name for client/server communication
     * @param handle IN library handle
     * @return the name that must be used for the socket
     */
    const char *flom_handle_get_socket_name(const flom_handle_t *handle);

    
    
    /**
     * Set trace filename
     * @param handle IN/OUT library handle
     * @param value IN the new name for trace file
     */
    void flom_handle_set_trace_filename(
        flom_handle_t *handle, const char *value);



    /**
     * Get trace filename
     * @param handle IN library handle
     * @return the current value for trace_file properties
     */
    const char *flom_handle_get_trace_filename(const flom_handle_t *handle);


    
    /**
     * Set resource name (the name of the resource that can be locked and
     * unlocked)
     * @param handle IN/OUT library handle
     * @param value IN name of the resource that can be locked and unlocked
     * @return a reason code
     */
    int flom_handle_set_resource_name(flom_handle_t *handle,
                                      const char *value);


    
    /**
     * Get resource name (the name of the resource that can be locked and
     * unlocked)
     * @param handle IN library handle
     * @return the name of the resource that can be locked and unlocked
     */
    const char *flom_handle_get_resource_name(const flom_handle_t *handle);

    

    /**
     * Set resource_create boolean property
     * @param handle IN/OUT library handle
     * @param value IN new value of the attribute: if TRUE,
     *              @ref flom_handle_lock will create a new resource if it's
     *              not yet available; if FALSE, @ref flom_handle_lock will
     *              not create a new resource if it's not available
     */
    void flom_handle_set_resource_create(flom_handle_t *handle, int value);



    /**
     * Get resource_create boolean property
     * @param handle IN library handle
     * return the value value of the attribute: if TRUE, @ref flom_handle_lock
     * will create a new resource if it's not yet available; if FALSE,
     * @ref flom_handle_lock will not create a new resource if it's not
     * available
     */
    int flom_handle_get_resource_create(const flom_handle_t *handle);


    
    /**
     * Set resource_timeout property: how long a lock operation will wait if
     * the resource is locked by another client
     * @param handle IN/OUT library handle
     * @param value IN new value of the attribute: <BR>
     *        0: no wait <BR>
     *        >0: maximum number of milliseconds to wait <BR>
     *        <0: infinite number of milliseconds to wait
     */
    void flom_handle_set_resource_timeout(flom_handle_t *handle, int value);



    /**
     * Get resource_time property: how long a lock operation will wait if
     * the resource is locked by another client
     * @param handle IN library handle
     * @return the current value of the attribute: <BR>
     *        0: no wait <BR>
     *        >0: maximum number of milliseconds to wait <BR>
     *        <0: infinite number of milliseconds to wait
     */
    int flom_handle_get_resource_timeout(const flom_handle_t *handle);


    
    /**
     * Set resource_quantity property: how many numeric resources you will
     * lock when calling @ref flom_handle_lock
     * @param handle IN/OUT library handle
     * @param value IN the new value for the attribute
     */
    void flom_handle_set_resource_quantity(flom_handle_t *handle, int value);



    /**
     * Get resource_quantity property: how many numeric resources you will
     * lock when calling @ref flom_handle_lock
     * @param handle IN library handle
     * @return the current value of the attribute: <BR>
     */
    int flom_handle_get_resource_quantity(const flom_handle_t *handle);


    
    /**
     * Set resource idle lifespan property: how many milliseconds a resource
     * will be kept after the last locker released it
     * @param handle IN/OUT library handle
     * @param value IN new value for the property
     */
    void flom_handle_set_resource_idle_lifespan(flom_handle_t *handle,
                                                int value);



    /**
     * Get resource idle lifespan property: how many milliseconds a resource
     * will be kept after the last locker released it
     * @param handle IN library handle
     * @return the current value
     */
    int flom_handle_get_resource_idle_lifespan(const flom_handle_t *handle);


    
    /**
     * Set unicast address property: the IP address (or the name resolved
     * by the system) of the host that must be contacted to reach the flom
     * daemon (server)
     * @param handle IN/OUT library handle
     * @param value IN the new value for the property
     */
    void flom_handle_set_unicast_address(flom_handle_t *handle,
                                         const char *value);


    
    /**
     * Get unicast address property: the IP address (or the name resolved
     * by the system) of the host that must be contacted to reach the flom
     * daemon (server)
     * @param handle IN library handle
     * @return the current value
     */
    const char *flom_handle_get_unicast_address(const flom_handle_t *handle);



    /**
     * Set TCP/IP unicast port that must be used to contact the flom daemon
     * @param handle IN/OUT library handle
     * @param value IN new value for the property
     */
    void flom_handle_set_unicast_port(flom_handle_t *handle, int value);



    /**
     * Get TCP/IP unicast port that must be used to contact the flom daemon
     * @param handle IN library handle
     * @return the current value
     */
    int flom_handle_get_unicast_port(const flom_handle_t *handle);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_HANDLE_H */
