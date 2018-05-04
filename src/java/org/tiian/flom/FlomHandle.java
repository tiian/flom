/*
 * Copyright (c) 2013-2018, Christian Ferrari <tiian@users.sourceforge.net>
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



package org.tiian.flom;



import java.nio.ByteBuffer;



public class FlomHandle {
    static {
        System.loadLibrary("flom_java");
    }


    
    /**
     * This is the opaque wrapper of a flom_handle_t object used by the
     * native library
     */
    private ByteBuffer NativeHandler;



    /**
     * Checks the current object is not corrupted
     */
    private void nullCheck() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
    }



    /*
     * Checks the current object is not corrupted and the passed argument is
     * not null
     * @param value (Input) the string that must be checked
     */
    private void nullCheck(String value) throws FlomException {
        nullCheck();
        if (null == value)
            throw new FlomException(FlomErrorCodes.FLOM_RC_NULL_OBJECT);
    }


    
    /**
     * Create a new native flom_handle_t object and set NativeHandler
     * Called by class constructor
     */
    private native int newJNI();
    /**
     * Create a new object calling the native interface
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public FlomHandle() throws FlomException {
        int ReturnCode = newJNI();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }


    
    /**
     * Delete the native flom_handle_t object.
     * Called by free method
     */
    private native void deleteJNI();
    /**
     * Explicitly free the native object allocated by JNI wrapper
     */
    public void free() {
        if (null != NativeHandler) {
            deleteJNI();
            NativeHandler = null;
        }
    }
    /**
     * Release native object if finalization is executed and the program
     * forgot to call release method
     */
    protected void finalize() {
        free();
    }


    
    /**
     * Native method for lock
     */
    private native int lockJNI();
    /**
     * Lock the (logical) resource linked to this handle; the resource
     * MUST be unlocked using method
     * {@link org.tiian.flom.FlomHandle#unlock unlock}
     * (or {@link org.tiian.flom.FlomHandle#unlockRollback unlockRollback})
     * when the lock condition is no more necessary.
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public void lock() throws FlomException {
        nullCheck();
        
        int ReturnCode = lockJNI();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }



    /**
     * Native method for unlock
     */
    private native int unlockJNI();
    /**
     * Unlock the (logical) resource linked to this handle; the resource
     * MUST be previously locked using method
     * {@link org.tiian.flom.FlomHandle#lock lock}
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public void unlock() throws FlomException {
        nullCheck();
        
        int ReturnCode = unlockJNI();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }


    
    /**
     * Native method for unlock rollback
     */
    private native int unlockRollbackJNI();
    /**
     * Unlock the (logical) resource linked to this handle and rollback the
     * transactiona resource state; the resource MUST be previously locked
     * using method
     * {@link org.tiian.flom.FlomHandle#lock lock}
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public void unlockRollback() throws FlomException {
        nullCheck();
        
        int ReturnCode = unlockRollbackJNI();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }


    
    /**
     * Native method for getLockedElement
     */
    private native String getLockedElementJNI();
    /**
     * Get the name of the locked element if the resource is of type set;
     * this method throws an exception if the name of the locked element is
     * not available
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getLockedElement() throws FlomException {
        String ReturnString = null;
        nullCheck();
        
        if (null == (ReturnString = getLockedElementJNI()))
            throw new FlomException(
                FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE);
        return ReturnString;
    }


    
    /**
     * Native method for getDiscoveryAttempts
     */
    private native int getDiscoveryAttemptsJNI();
    /**
     * Get the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     * getMulticastAddress, getMulticastPort).
     * The current value can be altered using method
     * setDiscoveryAttempts
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getDiscoveryAttempts() throws FlomException {
        nullCheck();
        return getDiscoveryAttemptsJNI();
    }



    /**
     * Native method for setDiscoveryAttempts
     */
    private native int setDiscoveryAttemptsJNI(int value);
    /**
     * Set the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         setMulticastAddress, setMulticastPort).
     * The current value can be inspected using method
     *         getDiscoveryAttempts
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setDiscoveryAttempts(int value) throws FlomException {
        nullCheck();
        return setDiscoveryAttemptsJNI(value);
    }

    
    
    /**
     * Native method for getDiscoveryTimeout
     */
    private native int getDiscoveryTimeoutJNI();
    /**
     * Get the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * getMulticastAddress, getMulticastPort).
     * The current value can be altered using method
     * setDiscoveryTimeout
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getDiscoveryTimeout() throws FlomException {
        nullCheck();
        return getDiscoveryTimeoutJNI();
    }


    
    /**
     * Native method for setDiscoveryTimeout
     */
    private native int setDiscoveryTimeoutJNI(int value);
    /**
     * Set the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * setMulticastAddress, setMulticastPort).
     * The current value can be inspected using method
     * getDiscoveryTtl.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setDiscoveryTimeout(int value) throws FlomException {
        nullCheck();
        return setDiscoveryTimeoutJNI(value);
    }

    
    
    /**
     * Native method for getDiscoveryTtl
     */
    private native int getDiscoveryTtlJNI();
    /**
     * Get the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be altered using method setDiscoveryTtl
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getDiscoveryTtl() throws FlomException {
        nullCheck();
        return getDiscoveryTtlJNI();
    }
    

    
    /**
     * Native method for setDiscoveryTtl
     */
    private native int setDiscoveryTtlJNI(int value);
    /**
     * Set the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be inspected using method
     * getDiscoveryTtl.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setDiscoveryTtl(int value) throws FlomException {
        nullCheck();
        return setDiscoveryTtlJNI(value);
    }


    /**
     * Native method for getLockMode
     */
    private native int getLockModeJNI();
    /**
     * Get lock mode property: how a simple or hierarchical resource will
     * be locked when method lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be altered using method setLockMode.
     * The available lock modes are described by class FlomLockModes
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getLockMode() throws FlomException {
        nullCheck();
        return getLockModeJNI();
    }


        
    /**
     * Native method for setLockMode
     */
    private native int setLockModeJNI(int value);
    /**
     * Set lock mode property: how a simple or hierarchical resource will
     * be locked when method lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be inspected using method getLockMode
     * @param value (Input): the new value
     * The available lock modes are described by class FlomLockModes
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setLockMode(int value) throws FlomException {
        nullCheck();
        return setLockModeJNI(value);
    }


    
    /**
     * Native method for getMulticastAddress
     */
    private native String getMulticastAddressJNI();
    /**
     * Get the multicast address: the IP address (or a network name that
     * the system can resolve) of the IP multicast group that must be
     * contacted to reach FLoM daemon (server) using UDP/IP; see also
     *         getMulticastPort.
     * The current value can be altered using method
     *         setMulticastAddress.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getMulticastAddress() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getMulticastAddressJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setMulticastAddress
     */
    private native int setMulticastAddressJNI(String value);
    /**
     * Set the multicast address: the IP address (or a network name that
     * the system can resolve) of the IP multicast group that must be
     * contacted to reach FLoM daemon (server) using UDP/IP; see also
     * setMulticastPort.
     * The current value can be inspected using method
     * getMulticastAddress.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setMulticastAddress(String value) throws FlomException {
        nullCheck(value);
        return setMulticastAddressJNI(value);
    }

    
    
    /**
     * Native method for getMulticastPort
     */
    private native int getMulticastPortJNI();
    /**
     * Get the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also getMulticastAddress.
     * The current value can be altered using method setMulticastPort.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getMulticastPort() throws FlomException {
        nullCheck();
        return getMulticastPortJNI();
    }


    
    /**
     * Native method for setMulticastPort
     */
    private native int setMulticastPortJNI(int value);
    /**
     * Set the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also setMulticastAddress.
     * The current value can be inspected using method
     * getMulticastPort.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setMulticastPort(int value) throws FlomException {
        nullCheck();
        return setMulticastPortJNI(value);
    }


    
    /**
     * Native method for getNetworkInterface
     */
    private native String getNetworkInterfaceJNI();
    /**
     * Get the network interface that must be used for IPv6 link local
     * addresses
     * The current value can be altered using method
     * setNetworkInterface.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getNetworkInterface() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getNetworkInterfaceJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setNetworkInterface
     */
    private native int setNetworkInterfaceJNI(String value);
    /**
     * Set the network interface that must be used for IPv6 link local
     * addresses
     * The current value can be inspected using method
     * getNetworkInterface.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setNetworkInterface(String value) throws FlomException {
        nullCheck(value);
        return setNetworkInterfaceJNI(value);
    }

    
    
    /**
     * Native method for getResourceCreate
     */
    private native boolean getResourceCreateJNI();
    /**
     * Get the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also getMulticastAddress.
     * The current value can be altered using method setResourceCreate.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public boolean getResourceCreate() throws FlomException {
        nullCheck();
        return getResourceCreateJNI();
    }


    
    /**
     * Native method for setResourceCreate
     */
    private native int setResourceCreateJNI(boolean value);
    /**
     * Set "resource create" boolean property: it specifies if method
     * lock can create a new resource when the specified
     * one is not defined.
     * The current value can be inspected using method
     * getResourceCreate.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setResourceCreate(boolean value) throws FlomException {
        nullCheck();
        return setResourceCreateJNI(value);
    }


    
    /**
     * Native method for getResourceIdleLifespan
     */
    private native int getResourceIdleLifespanJNI();
    /**
     * Get "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released
     * it; the expiration is necessary to avoid useless resource allocation.
     * The current value can be altered using method
     *     setResourceIdleLifespan.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getResourceIdleLifespan() throws FlomException {
        nullCheck();
        return getResourceIdleLifespanJNI();
    }


    
    /**
     * Native method for setResourceIdleLifespan
     */
    private native int setResourceIdleLifespanJNI(int value);
    /**
     * Set "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released
     * it; the expiration is necessary to avoid useless resource allocation.
     * The current value can be inspected using method
     *     getResourceIdleLifespan.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setResourceIdleLifespan(int value) throws FlomException {
        nullCheck();
        return setResourceIdleLifespanJNI(value);
    }


    
    /**
     * Native method for getResourceName
     */
    private native String getResourceNameJNI();
    /**
     * Get the resource name: the name of the resource that can be locked
     * and unlocked using lock and unlock methods.
     * The current value can be altered using method setResourceName.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getResourceName() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getResourceNameJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setResourceName
     */
    private native int setResourceNameJNI(String value);
    /**
     * Set the resource name: the name of the resource that can be locked
     * and unlocked using lock and unlock methods.
     * The current value can be inspected using method getResourceName.
     * NOTE: the resource type is determined by its name; take a look to
     * flom command man page (-r, --resource-name option) for an
     * explanation of the resource name grammar.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setResourceName(String value) throws FlomException {
        int ReturnCode = FlomErrorCodes.FLOM_RC_OK;
        nullCheck(value);
        ReturnCode = setResourceNameJNI(value);
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode &&
            FlomErrorCodes.FLOM_RC_API_IMMUTABLE_HANDLE != ReturnCode)
            throw new FlomException(ReturnCode);
        return ReturnCode;
    }

    
    
    /**
     * Native method for getResourceQuantity
     */
    private native int getResourceQuantityJNI();
    /**
     * Get "resource quantity" property: the number of units that will be
     * locked and unlocked using lock and unlock methods.
     * The current value can be altered using method
     * setResourceQuantity.
     * NOTE: this property applies to "numeric resources" only.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getResourceQuantity() throws FlomException {
        nullCheck();
        return getResourceQuantityJNI();
    }


    
    /**
     * Native method for setResourceQuantity
     */
    private native int setResourceQuantityJNI(int value);
    /**
     * Set "resource quantity" property: the number of units that will be
     * locked and unlocked using lock and unlock methods.
     * The current value can be inspected using method
     * getResourceQuantity.
     * NOTE: this property applies to "numeric resources" only.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setResourceQuantity(int value) throws FlomException {
        nullCheck();
        return setResourceQuantityJNI(value);
    }


    
    /**
     * Native method for getResourceTimeout
     */
    private native int getResourceTimeoutJNI();
    /**
     * Get "resource timeout" property: how long a lock operation
     * (see lock) will wait if the resource is locked
     * by another requester.
     * The current value can be altered using method
     * setResourceTimeout.
     * @return the current value: <BR>
     *        0: no wait <BR>
     *        &gt; 0: maximum number of milliseconds to wait <BR>
     *        &lt; 0: unlimited wait
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getResourceTimeout() throws FlomException {
        nullCheck();
        return getResourceTimeoutJNI();
    }


    
    /**
     * Native method for setResourceTimeout
     */
    private native int setResourceTimeoutJNI(int value);
    /**
     * Set "resource timeout" property: how long a lock operation
     * (see lock) will wait if the resource is locked
     * by another requester.
     * The current value can be inspected using method
     * getResourceTimeout.
     * @param value (Input): the new value: <BR>
     *        0: no wait <BR>
     *        &gt; 0: maximum number of milliseconds to wait <BR>
     *        &lt; 0: unlimited wait
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setResourceTimeout(int value) throws FlomException {
        nullCheck();
        return setResourceTimeoutJNI(value);
    }


    
    /**
     * Native method for getSocketName
     */
    private native String getSocketNameJNI();
    /**
     * Get the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be altered using method setSocketName.
     * @return the current value as a standard
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getSocketName() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getSocketNameJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setSocketName
     */
    private native int setSocketNameJNI(String value);
    /**
     * Set the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be inspected using method getSocketName
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setSocketName(String value) throws FlomException {
        nullCheck(value);
        return setSocketNameJNI(value);
    }

    
    
    /**
     * Native method for getTraceFilename
     */
    private native String getTraceFilenameJNI();
    /**
     * Get the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be altered using method setTraceFilename.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getTraceFilename() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getTraceFilenameJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setTraceFilename
     */
    private native int setTraceFilenameJNI(String value);
    /**
     * Set the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be inspected using method
     * getTraceFilename.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setTraceFilename(String value) throws FlomException {
        nullCheck(value);
        return setTraceFilenameJNI(value);
    }

    
    
    /**
     * Native method for getUnicastAddress
     */
    private native String getUnicastAddressJNI();
    /**
     * Get the unicast address: the IP address (or a network name that the
     * system can resolve) of the host that must be contacted
     * to reach FLoM daemon (server) using TCP/IP; see also
     *         getUnicastPort.
     * The current value can be altered using method
     *         setUnicastAddress.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getUnicastAddress() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getUnicastAddressJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setUnicastAddress
     */
    private native int setUnicastAddressJNI(String value);
    /**
     * Set the unicast address: the IP address (or a network name that the
     * system can resolve) of the host that must be contacted
     * to reach FLoM daemon (server) using TCP/IP; see also
     * setUnicastPort.
     * The current value can be inspected using method
     * getUnicastAddress.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setUnicastAddress(String value) throws FlomException {
        nullCheck(value);
        return setUnicastAddressJNI(value);
    }

    
    
    /**
     * Native method for getUnicastPort
     */
    private native int getUnicastPortJNI();
    /**
     * Get the TCP/IP unicast port that must be used to contact the FLoM
     * daemon (server) using TCP/IP; see also getUnicastAddress.
     * The current value can be altered using method setUnicastPort.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int getUnicastPort() throws FlomException {
        nullCheck();
        return getUnicastPortJNI();
    }


    
    /**
     * Native method for setUnicastPort
     */
    private native int setUnicastPortJNI(int value);
    /**
     * Set the TCP/IP unicast port that must be used to contact the FLoM
     * daemon (server) using TCP/IP; see also setUnicastAddress.
     * The current value can be inspected using method getUnicastPort.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setUnicastPort(int value) throws FlomException {
        nullCheck();
        return setUnicastPortJNI(value);
    }


    
    /**
     * Native method for getTlsCertificate
     */
    private native String getTlsCertificateJNI();
    /**
     * Get the TLS certificate file name.
     * The current value can be altered using method
     *         setTlsCertificate.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getTlsCertificate() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getTlsCertificateJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setTlsCertificate
     */
    private native int setTlsCertificateJNI(String value);
    /**
     * Set the TLS certificate file name.
     * The current value can be inspected using method
     * getTlsCertificate.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setTlsCertificate(String value) throws FlomException {
        nullCheck(value);
        return setTlsCertificateJNI(value);
    }


    
    /**
     * Native method for getTlsPrivateKey
     */
    private native String getTlsPrivateKeyJNI();
    /**
     * Get the TLS private key file name.
     * The current value can be altered using method
     *         setTlsPrivateKey.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getTlsPrivateKey() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getTlsPrivateKeyJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setTlsPrivateKey
     */
    private native int setTlsPrivateKeyJNI(String value);
    /**
     * Set the TLS CA private key file name.
     * The current value can be inspected using method
     * getTlsPrivateKey.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setTlsPrivateKey(String value) throws FlomException {
        nullCheck(value);
        return setTlsPrivateKeyJNI(value);
    }


    
    /**
     * Native method for getTlsCaCertificate
     */
    private native String getTlsCaCertificateJNI();
    /**
     * Get the TLS CA certificate file name.
     * The current value can be altered using method
     *         setTlsCaCertificate.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public String getTlsCaCertificate() throws FlomException {
        String ReturnString = null;
        nullCheck();
        if (null == (ReturnString = getTlsCaCertificateJNI()))
            ReturnString = new String("");
        return ReturnString;
    }


    
    /**
     * Native method for setTlsCaCertificate
     */
    private native int setTlsCaCertificateJNI(String value);
    /**
     * Set the TLS CA certificate file name.
     * The current value can be inspected using method
     * getTlsCaCertificate.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setTlsCaCertificate(String value) throws FlomException {
        nullCheck(value);
        return setTlsCaCertificateJNI(value);
    }



    /**
     * Native method for getTlsCheckPeerId
     */
    private native boolean getTlsCheckPeerIdJNI();
    /**
     * Get the TLS check peer ID flag value.
     * The current value can be altered using method setTlsCheckPeerId.
     * @return the current value
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public boolean getTlsCheckPeerId() throws FlomException {
        nullCheck();
        return getTlsCheckPeerIdJNI();
    }


    
    /**
     * Native method for setTlsCheckPeerId
     */
    private native int setTlsCheckPeerIdJNI(boolean value);
    /**
     * Set "TLS check peer ID" boolean property: it specifies if this node
     * must check the unique ID presented by the remote peer.
     * The current value can be inspected using method
     * getTlsCheckPeerId.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     * @throws FlomException if the underlying native C function returns
     * an error condition
     */
    public int setTlsCheckPeerId(boolean value) throws FlomException {
        nullCheck();
        return setTlsCheckPeerIdJNI(value);
    }    
}