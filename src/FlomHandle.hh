/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FLOMHANDLE_HH
# define FLOMHANDLE_HH



#include <string>
#include <exception>
#include <syslog.h>



#include "flom_handle.h"



using namespace std;

namespace flom {
    
    /**
     * FLoM exception class, it extends standard exception class and adds the
     * return code property that maps on C API
     */
    class FlomException : public exception {
        virtual const char* what() const throw()
        {
            return flom_strerror(ReturnCode);
        }

        private:
        /**
         * Return code returned by the failed C function
         */
        int ReturnCode;

        public:
        /**
         * @param ret_cod: the return code of the failed C function
         */
        FlomException(int ret_cod) {
            ReturnCode = ret_cod; }
        /**
         * Retrieve the numeric return code of the failed C function
         */
        int getReturnCode() { return ReturnCode; }
        /**
         * Retrieve the description associated to the numeric return code of
         * the failed C function
         */
        string getReturnCodeText() {
            return (string(flom_strerror(ReturnCode))); }
    };



    /**
     * This class provides C++ abstraction to C flom_handle_t type
     */
    class FlomHandle {
        private:
        /**
         * C FLoM handle object
         */
        flom_handle_t handle;

        public:
        FlomHandle() {
            int ret_cod = flom_handle_init(&handle);
            /* in case of error, an exception is thrown */
            if (FLOM_RC_OK != ret_cod) {
                syslog(LOG_ERR, "FlomHandle/flom_handle_init: "
                       "ret_cod=%d ('%s')\n", ret_cod, flom_strerror(ret_cod));
                throw FlomException(ret_cod);
            }
        }
        ~FlomHandle() {
            int ret_cod = flom_handle_clean(&handle);
            /* exception can NOT be thrown from a destructor, only syslog
               records the issue */
            if (FLOM_RC_OK != ret_cod) {
                syslog(LOG_ERR, "~FlomHandle/flom_handle_clean: "
                       "ret_cod=%d ('%s')\n", ret_cod, flom_strerror(ret_cod));
            }
        }

        /**
         * Locks the (logical) resource linked to this handle; the resource
         * MUST be unlocked using method @ref unlock when the lock condition
         * is no more necessary.
         * @return a reason code (see file @ref flom_errors.h)
         */
        int lock() { return flom_handle_lock(&handle); }
        
        /**
         * Unlocks the (logical) resource linked to this handle; the resource
         * MUST be previously locked using method @ref lock
         * @return a reason code (see file @ref flom_errors.h)
         */
        int unlock() { return flom_handle_unlock(&handle); }

        /**
         * Gets the name of the locked element if the resource is of
         * type set.<P>
         * Note 1: this method can be used only after @ref lock and before
         *         @ref unlock<P>
         * Note 2: this method can be used only when locking a resource of
         *         type "resource set"<P>
         * Note 3: the return string must copied as soon as possible to a
         *         different place because it's a dynamic string removed by
         *         @ref unlock<P>
         * @return the name of the locked element
         */
        const char *getLockedElementAsCStr() {
            return flom_handle_get_locked_element(&handle); }
        
        /**
         * Gets the name of the locked element if the resource is of
         * type set.<P>
         * Note 1: this method can be used only after @ref lock and before
         *         @ref unlock<P>
         * Note 2: this method can be used only when locking a resource of
         *         type "resource set"<P>
         * Note 3: the return string must copied as soon as possible to a
         *         different place because it's a dynamic string removed by
         *         @ref unlock<P>
         * @return the name of the locked element
         */
        string getLockedElement() {
            return NULL != flom_handle_get_locked_element(&handle) ?
                flom_handle_get_locked_element(&handle) : ""; }
        
        /**
         * Gets the maximum number of attempts that will be tryed during
         * auto-discovery phase using UDP/IP multicast (see
         *         @ref getMulticastAddress, @ref getMulticastPort).
         * The current value can be altered using method
         * @ref setDiscoveryAttempts
         * @return the current value
         */
        int getDiscoveryAttempts() {
            return flom_handle_get_discovery_attempts(&handle); }

        /**
         * Sets the maximum number of attempts that will be tryed during
         * auto-discovery phase using UDP/IP multicast (see
         *         @ref setMulticastAddress, @ref setMulticastPort).
         * The current value can be inspected using method
         *         @ref getDiscoveryAttempts
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setDiscoveryAttempts(int value) {
            return flom_handle_set_discovery_attempts(&handle, value); }

        /**
         * Gets the number of milliseconds between two consecutive attempts
         * that will be tryed during auto-discovery phase using UDP/IP
         * multicast (see
         * @ref getMulticastAddress, @ref getMulticastPort).
         * The current value can be altered using method
         * @ref setDiscoveryTimeout
         * @return the current value
         */
        int getDiscoveryTimeout() {
            return flom_handle_get_discovery_timeout(&handle); }

        /**
         * Sets the number of milliseconds between two consecutive attempts
         * that will be tryed during auto-discovery phase using UDP/IP
         * multicast (see
         * @ref setMulticastAddress, @ref setMulticastPort).
         * The current value can be inspected using method
         * @ref getDiscoveryTimeout.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setDiscoveryTimeout(int value) {
            return flom_handle_set_discovery_timeout(&handle, value); }

        /**
         * Gets the UDP/IP multicast TTL parameter used during auto-discovery
         * phase; for a definition of the parameter, see
         * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
         * . The current value can be altered using method @ref setDiscoveryTtl
         * @return the current value
         */
        int getDiscoveryTtl() {
            return flom_handle_get_discovery_ttl(&handle); }

        /**
         * Sets the UDP/IP multicast TTL parameter used during auto-discovery
         * phase; for a definition of the parameter, see
         * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html 
         * . The current value can be inspected using method
         * @ref getDiscoveryTtl.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setDiscoveryTtl(int value) {
            return flom_handle_set_discovery_ttl(&handle, value); }

        /**
         * Gets lock mode property: how a simple or hierarchical resource will
         * be locked when method @ref lock is called; FLoM
         * supports the same lock mode semantic proposed by DLM, see
         * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
         * for a detailed explanation
         * . The current value can be altered using method @ref setLockMode
         * @return the current value
         */
        flom_lock_mode_t getLockMode() {
            return flom_handle_get_lock_mode(&handle); }

        /**
         * Sets lock mode property: how a simple or hierarchical resource will
         * be locked when method @ref lock is called; FLoM
         * supports the same lock mode semantic proposed by DLM, see
         * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
         * for a detailed explanation
         * . The current value can be inspected using method @ref getLockMode
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setLockMode(flom_lock_mode_t value) {
            return flom_handle_set_lock_mode(&handle, value); }

        /**
         * Gets the multicast address: the IP address (or a network name that
         * the system can resolve) of the IP multicast group that must be
         * contacted to reach FLoM daemon (server) using UDP/IP; see also
         *         @ref getMulticastPort.
         * The current value can be altered using method
         *         @ref setMulticastAddress.
         * @return the current value as a null terminated C string
         */
        const char *getMulticastAddressAsCStr() {
            return flom_handle_get_multicast_address(&handle); }
        /**
         * Gets the multicast address: the IP address (or a network name that
         * the system can resolve) of the IP multicast group that must be
         * contacted to reach FLoM daemon (server) using UDP/IP; see also
         *         @ref getMulticastPort.
         * The current value can be altered using method
         *         @ref setMulticastAddress.
         * @return the current value as a standard C++ string
         */
        string getMulticastAddress() {
            return NULL != flom_handle_get_multicast_address(&handle) ?
                flom_handle_get_multicast_address(&handle) : ""; }

        /**
         * Sets the multicast address: the IP address (or a network name that
         * the system can resolve) of the IP multicast group that must be
         * contacted to reach FLoM daemon (server) using UDP/IP; see also
         * @ref setMulticastPort.
         * The current value can be inspected using method
         * @ref getMulticastAddress.
         * @param value (Input): the new value (C null terminated string)
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setMulticastAddress(const char *value) {
            return flom_handle_set_multicast_address(&handle, value); }
        /**
         * Sets the multicast address: the IP address (or a network name that
         * the system can resolve) of the IP multicast group that must be
         * contacted to reach FLoM daemon (server) using UDP/IP; see also
         * @ref setMulticastPort.
         * The current value can be inspected using method
         * @ref getMulticastAddress.
         * @param value (Input): the new value (C++ standard string)
         */
        void setMulticastAddress(const string &value) {
            flom_handle_set_multicast_address(&handle, value.c_str()); }

        /**
         * Gets the UDP/IP multicast port that must be used to contact the FLoM
         * daemon (server) using UDP/IP; see also @ref getMulticastAddress.
         * The current value can be altered using method @ref setMulticastPort.
         * @return the current value
         */
        int getMulticastPort() {
            return flom_handle_get_multicast_port(&handle); }

        /**
         * Sets the UDP/IP multicast port that must be used to contact the FLoM
         * daemon (server) using UDP/IP; see also @ref setMulticastAddress.
         * The current value can be inspected using method
         * @ref getMulticastPort.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setMulticastPort(int value) {
            return flom_handle_set_multicast_port(&handle, value); }

        /**
         * Gets "resource create" boolean property: it specifies if method
         * @ref lock can create a new resource when the specified
         * one is not defined; the default value is TRUE. 
         * The current value can be altered using method 
         *     @ref setResourceCreate.
         * @return the current value
         */
        int getResourceCreate() {
            return flom_handle_get_resource_create(&handle); }

        /**
         * Sets "resource create" boolean property: it specifies if method
         * @ref lock can create a new resource when the specified
         * one is not defined.
         * The current value can be inspected using method
         * @ref getResourceCreate.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceCreate(int value) {
            return flom_handle_set_resource_create(&handle, value); }

        /**
         * Gets "resource idle lifespan" property: it specifies how many
         * milliseconds a resource will be kept after the last locker released
         * it; the expiration is necessary to avoid useless resource
         * allocation.
         * The current value can be altered using method
         *     @ref setResourceIdleLifespan.
         * @return the current value
         */
        int getResourceIdleLifespan() {
            return flom_handle_get_resource_idle_lifespan(&handle); }

        /**
         * Sets "resource idle lifespan" property: it specifies how many
         * milliseconds a resource will be kept after the last locker released
         * it; the expiration is necessary to avoid useless resource
         * allocation.
         * The current value can be inspected using method
         *     @ref getResourceIdleLifespan.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceIdleLifespan(int value) {
            return flom_handle_set_resource_idle_lifespan(&handle, value); }

        /**
         * Gets the resource name: the name of the resource that can be locked
         * and unlocked using @ref lock and @ref unlock methods.
         * The current value can be altered using method @ref setResourceName.
         * @return the current value as a null terminated C string
         */
        const char *getResourceNameAsCStr() {
            return flom_handle_get_resource_name(&handle); }
        /**
         * Gets the resource name: the name of the resource that can be locked
         * and unlocked using @ref lock and @ref unlock methods.
         * The current value can be altered using method @ref setResourceName.
         * @return the current value as a C++ standard string
         */
        string getResourceName() {
            return NULL != flom_handle_get_resource_name(&handle) ?
                flom_handle_get_resource_name(&handle) : ""; }

        /**
         * Sets the resource name: the name of the resource that can be locked
         * and unlocked using @ref lock and @ref unlock methods.
         * The current value can be inspected using method
         * @ref getResourceName.
         * NOTE: the resource type is determined by its name; take a look to
         * flom command man page (-r, --resource-name option) for an
         * explanation of the resource name grammar.
         * @param value (Input): the new value (C null terminated string)
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceName(const char *value) {
            return flom_handle_set_resource_name(&handle, value); }
        /**
         * Sets the resource name: the name of the resource that can be locked
         * and unlocked using @ref lock and @ref unlock methods.
         * The current value can be inspected using method
         * @ref getResourceName.
         * NOTE: the resource type is determined by its name; take a look to
         * flom command man page (-r, --resource-name option) for an
         * explanation of the resource name grammar.
         * @param value (Input): the new value (C++ standard string)
         * @return a reason code (see file @ref flom_errors.h)
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceName(const string &value) {
            return flom_handle_set_resource_name(&handle, value.c_str()); }

        /**
         * Gets "resource quantity" property: the number of units that will be
         * locked and unlocked using @ref lock and @ref unlock methods.
         * The current value can be altered using method
         * @ref setResourceQuantity.
         * NOTE: this property applies to "numeric resources" only.
         * @return the current value
         */
        int getResourceQuantity() {
            return flom_handle_get_resource_quantity(&handle); }

        /**
         * Sets "resource quantity" property: the number of units that will be
         * locked and unlocked using @ref lock and @ref unlock methods.
         * The current value can be inspected using method
         * @ref getResourceQuantity.
         * NOTE: this property applies to "numeric resources" only.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceQuantity(int value) {
            return flom_handle_set_resource_quantity(&handle, value); }

        /**
         * Gets "resource timeout" property: how long a lock operation
         * (see @ref lock) will wait if the resource is locked
         * by another requester.
         * The current value can be altered using method
         * @ref setResourceTimeout.
         * @return the current value: <BR>
         *        0: no wait <BR>
         *        >0: maximum number of milliseconds to wait <BR>
         *        <0: unlimited wait
         */
        int getResourceTimeout() {
            return flom_handle_get_resource_timeout(&handle); }

        /**
         * Sets "resource timeout" property: how long a lock operation
         * (see @ref lock) will wait if the resource is locked
         * by another requester.
         * The current value can be inspected using method
         * @ref getResourceTimeout.
         * @param value (Input): the new value: <BR>
         *        0: no wait <BR>
         *        >0: maximum number of milliseconds to wait <BR>
         *        <0: unlimited wait
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setResourceTimeout(int value) {
            return flom_handle_set_resource_timeout(&handle, value); }

        /**
         * Gets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
         * used to contact a local FLoM daemon (server).
         * The current value can be altered using method @ref setSocketName.
         * @return the current value as a C null terminated string
         */
        const char *getSocketNameAsCStr() {
            return flom_handle_get_socket_name(&handle); }
        /**
         * Gets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
         * used to contact a local FLoM daemon (server).
         * The current value can be altered using method @ref setSocketName.
         * @return the current value as a C++ standard string
         */
        string getSocketName() {
            return NULL != flom_handle_get_socket_name(&handle) ?
                flom_handle_get_socket_name(&handle) : ""; }

        /**
         * Sets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
         * used to contact a local FLoM daemon (server).
         * The current value can be inspected using method @ref getSocketName.
         * @param value (Input): the new value (C null terminated string)
         * @return a reason code (see file @ref flom_errors.h)
         */
        int setSocketName(const char *value) {
            return flom_handle_set_socket_name(&handle, value); }
        /**
         * Sets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
         * used to contact a local FLoM daemon (server).
         * The current value can be inspected using method @ref getSocketName.
         * @param value (Input): the new value (C++ standard string)
         * @return a reason code (see file @ref flom_errors.h)
         */
        int setSocketName(const string &value) {
            return flom_handle_set_socket_name(&handle, value.c_str()); }

        /**
         * Gets the trace filename: the name (absolute or relative path) used
         * by libflom (FLoM client library) to record trace messages.
         * The current value can be altered using method @ref setTraceFilename.
         * @return the current value as a C null terminated string
         */
        const char *getTraceFilenameAsCStr() {
            return flom_handle_get_trace_filename(&handle); }
        /**
         * Gets the trace filename: the name (absolute or relative path) used
         * by libflom (FLoM client library) to record trace messages.
         * The current value can be altered using method @ref setTraceFilename.
         * @return the current value as a C++ standard string
         */
        string getTraceFilename() {
            return NULL != flom_handle_get_trace_filename(&handle) ?
                flom_handle_get_trace_filename(&handle) : ""; }

        /**
         * Sets the trace filename: the name (absolute or relative path) used
         * by libflom (FLoM client library) to record trace messages.
         * The current value can be inspected using method
         * @ref getTraceFilename.
         * @param value (Input): the new value (C null terminated string)
         * @return @ref FLOM_RC_OK
         */
        int setTraceFilename(const char *value) {
            return flom_handle_set_trace_filename(&handle, value); }
        /**
         * Sets the trace filename: the name (absolute or relative path) used
         * by libflom (FLoM client library) to record trace messages.
         * The current value can be inspected using method
         * @ref getTraceFilename.
         * @param value (Input): the new value (C++ standard string)
         * @return @ref FLOM_RC_OK
         */
        int setTraceFilename(const string &value) {
            return flom_handle_set_trace_filename(&handle, value.c_str()); }

        /**
         * Gets the unicast address: the IP address (or a network name that the
         * system can resolve) of the host that must be contacted
         * to reach FLoM daemon (server) using TCP/IP; see also
         * @ref getUnicastPort.
         * The current value can be altered using method
         * @ref setUnicastAddress.
         * @return the current value as a C null terminated string
         */
        const char *getUnicastAddressAsCStr() {
            return flom_handle_get_unicast_address(&handle); }
        /**
         * Gets the unicast address: the IP address (or a network name that the
         * system can resolve) of the host that must be contacted
         * to reach FLoM daemon (server) using TCP/IP; see also
         * @ref getUnicastPort.
         * The current value can be altered using method
         * @ref setUnicastAddress.
         * @return the current value as a C++ standard string
         */
        string getUnicastAddress() {
            return NULL != flom_handle_get_unicast_address(&handle) ?
                flom_handle_get_unicast_address(&handle) : ""; }

        /**
         * Sets the unicast address: the IP address (or a network name that the
         * system can resolve) of the host that must be contacted
         * to reach FLoM daemon (server) using TCP/IP; see also
         * @ref setUnicastPort.
         * The current value can be inspected using method
         * @ref getUnicastAddress.
         * @param value (Input): the new value (C null terminted string)
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setUnicastAddress(const char *value) {
            return flom_handle_set_unicast_address(&handle, value); }
        /**
         * Sets the unicast address: the IP address (or a network name that the
         * system can resolve) of the host that must be contacted
         * to reach FLoM daemon (server) using TCP/IP; see also
         * @ref setUnicastPort.
         * The current value can be inspected using method
         * @ref getUnicastAddress.
         * @param value (Input): the new value (C++ standard string)
         */
        void setUnicastAddress(const string &value) {
            flom_handle_set_unicast_address(&handle, value.c_str()); }

        /**
         * Gets the TCP/IP unicast port that must be used to contact the FLoM
         * daemon (server) using TCP/IP; see also @ref getUnicastAddress.
         * The current value can be altered using method @ref setUnicastPort.
         * @return the current value
         */
        int getUnicastPort() {
            return flom_handle_get_unicast_port(&handle); }

        /**
         * Sets the TCP/IP unicast port that must be used to contact the FLoM
         * daemon (server) using TCP/IP; see also @ref setUnicastAddress.
         * The current value can be inspected using method @ref getUnicastPort.
         * @param value (Input): the new value
         * @return @ref FLOM_RC_OK or @ref FLOM_RC_API_IMMUTABLE_HANDLE
         */
        int setUnicastPort(int value) {
            return flom_handle_set_unicast_port(&handle, value); }
    }; /* class FlomHandle */

} /* namespace flom */



#endif /* FLOMHANDLE_HH */
