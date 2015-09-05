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
     * This field is used to pass the native return code from native to Java
     * when an exception must be propagated: native generates only standard
     * Exception objects, while Java generates custom FlomException objects
     */
    /* @@@ probably useless, remove me!!!
    private int NativeReturnCode;
    */
    

    
    /**
     * Create a new native @ref flom_handle_t object and set @ref NativeHandler
     * Called by class constructor
     */
    private native int newJNI();
    /**
     * Create a new object calling the native interface
     */
    public FlomHandle() throws FlomException {
        int ReturnCode = newJNI();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }


    
    /**
     * Delete the native @ref flom_handle_t object
     * Called by @ref free method
     */
    private native int deleteJNI();
    /**
     * Explicitly free the native object allocated by JNI wrapper
     */
    public void free() throws FlomException {
        if (null != NativeHandler) {
            int ReturnCode = deleteJNI();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
            NativeHandler = null;
        }
    }
    /**
     * Release native object if finalization is executed and the program
     * forgot to call @ref release method
     */
    protected void finalize() {
        try {
            free();
        } catch(FlomException e) {
            System.err.println("FlomHandle.finalize() thrown a " +
                               "FlomException: ReturnCode=" +
                               e.getReturnCode() + 
                               " (" + e.getMessage() + ")");
        }
    }


    
    /**
     * Native method for @ref lock
     */
    private native int lockJNI();
    /**
     * Lock the (logical) resource linked to this handle; the resource
     * MUST be unlocked using method @ref unlock when the lock condition
     * is no more necessary.  Use this instance of the method if you are
     * interested to the name of the locked element and you prefer a C
     * null terminated string.
     */
    public void lock() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            int ReturnCode = lockJNI();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
        }
    }



    /**
     * Native method for @ref unlock
     */
    private native int unlockJNI();
    /**
     * Unlock the (logical) resource linked to this handle; the resource
     * MUST be previously locked using method @ref lock
     */
    public void unlock() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            int ReturnCode = unlockJNI();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
        }
    }


    
    /**
     * Native method for @ref getLockedElement
     */
    private native String getLockedElementJNI();
    /**
     * Get the name of the locked element if the resource is of type set;
     * this method throws an exception if the name of the locked element is
     * not available
     */
    public String getLockedElement() throws FlomException {
        String ReturnString = null;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            if (null == (ReturnString = getLockedElementJNI()))
                throw new FlomException(
                    FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE);
        }
        return ReturnString;
    }


    
    /**
     * Native method for @ref getDiscoveryAttempts
     */
    private native int getDiscoveryAttemptsJNI();
    /**
     * Get the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     * @ref getMulticastAddress, @ref getMulticastPort).
     * The current value can be altered using method
     * @ref setDiscoveryAttempts
     * @return the current value
     */
    public int getDiscoveryAttempts() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getDiscoveryAttemptsJNI();
    }



    /**
     * Native method for @ref setDiscoveryAttempts
     */
    private native int setDiscoveryAttemptsJNI(int value);
    /**
     * Sets the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         @ref setMulticastAddress, @ref setMulticastPort).
     * The current value can be inspected using method
     *         @ref getDiscoveryAttempts
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setDiscoveryAttempts(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setDiscoveryAttemptsJNI(value);
    }

    
    
    /**
     * Native method for @ref getDiscoveryTimeout
     */
    private native int getDiscoveryTimeoutJNI();
    /**
     * Gets the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * @ref getMulticastAddress, @ref getMulticastPort).
     * The current value can be altered using method
     * @ref setDiscoveryTimeout
     * @return the current value
     */
    public int getDiscoveryTimeout() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getDiscoveryTimeoutJNI();
    }


    
    /**
     * Native method for @ref setDiscoveryTimeout
     */
    private native int setDiscoveryTimeoutJNI(int value);
    /**
     * Sets the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * @ref setMulticastAddress, @ref setMulticastPort).
     * The current value can be inspected using method
     * @ref getDiscoveryTtl.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setDiscoveryTimeout(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setDiscoveryTimeoutJNI(value);
    }

    
    
    /**
     * Native method for @ref getDiscoveryTtl
     */
    private native int getDiscoveryTtlJNI();
    /**
     * Gets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be altered using method @ref setDiscoveryTtl
     * @return the current value
     */
    public int getDiscoveryTtl() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getDiscoveryTtlJNI();
    }
    

    
    /**
     * Native method for @ref setDiscoveryTtl
     */
    private native int setDiscoveryTtlJNI(int value);
    /**
     * Sets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be inspected using method
     * @ref getDiscoveryTtl.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setDiscoveryTtl(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setDiscoveryTtlJNI(value);
    }


    /**
     * Native method for @ref getLockMode
     */
    private native int getLockModeJNI();
    /**
     * Gets lock mode property: how a simple or hierarchical resource will
     * be locked when method @ref lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be altered using method @ref setLockMode.
     * The available lock modes are described by class @ref FlomLockModes
     * @return the current value
     */
    public int getLockMode() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getLockModeJNI();
    }


        
    /**
     * Native method for @ref setLockMode
     */
    private native int setLockModeJNI(int value);
    /**
     * Sets lock mode property: how a simple or hierarchical resource will
     * be locked when method @ref lock is called; FLoM
     * supports the same lock mode semantic proposed by DLM, see
     * http://en.wikipedia.org/wiki/Distributed_lock_manager#Lock_modes
     * for a detailed explanation
     * . The current value can be inspected using method @ref getLockMode
     * @param value (Input): the new value
     * The available lock modes are described by class @ref FlomLockModes
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setLockMode(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setLockModeJNI(value);
    }


    
    /**
     * Native method for @ref getMulticastAddress
     */
    private native String getMulticastAddressJNI();
    /**
     * Gets the multicast address: the IP address (or a network name that
     * the system can resolve) of the IP multicast group that must be
     * contacted to reach FLoM daemon (server) using UDP/IP; see also
     *         @ref getMulticastPort.
     * The current value can be altered using function
     *         @ref setMulticastAddress.
     * @return the current value
     */
    public String getMulticastAddress() throws FlomException {
        String ReturnString = null;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            if (null == (ReturnString = getMulticastAddressJNI()))
                ReturnString = new String("");
        }
        return ReturnString;
    }


    
    /**
     * Native method for @ref setMulticastAddress
     */
    private native int setMulticastAddressJNI(String value);
    /**
     * Sets the multicast address: the IP address (or a network name that
     * the system can resolve) of the IP multicast group that must be
     * contacted to reach FLoM daemon (server) using UDP/IP; see also
     * @ref setMulticastPort.
     * The current value can be inspected using method
     * @ref getMulticastAddress.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setMulticastAddress(String value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else if (null == value)
            throw new FlomException(FlomErrorCodes.FLOM_RC_NULL_OBJECT);
        else
            return setMulticastAddressJNI(value);
    }

    
    
    /**
     * Native method for @ref getMulticastPort
     */
    private native int getMulticastPortJNI();
    /**
     * Gets the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also @ref getMulticastAddress.
     * The current value can be altered using method @ref setMulticastPort.
     * @return the current value
     */
    public int getMulticastPort() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getMulticastPortJNI();
    }


    
    /**
     * Native method for @ref setMulticastPort
     */
    private native int setMulticastPortJNI(int value);
    /**
     * Sets the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also @ref setMulticastAddress.
     * The current value can be inspected using method
     * @ref getMulticastPort.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setMulticastPort(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setMulticastPortJNI(value);
    }


    
    /**
     * Native method for @ref getResourceCreate
     */
    private native boolean getResourceCreateJNI();
    /**
     * Gets the UDP/IP multicast port that must be used to contact the FLoM
     * daemon (server) using UDP/IP; see also @ref getMulticastAddress.
     * The current value can be altered using method @ref setResourceCreate.
     * @return the current value
     */
    public boolean getResourceCreate() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getResourceCreateJNI();
    }


    
    /**
     * Native method for @ref setResourceCreate
     */
    private native int setResourceCreateJNI(boolean value);
    /**
     * Sets "resource create" boolean property: it specifies if method
     * @ref lock can create a new resource when the specified
     * one is not defined.
     * The current value can be inspected using method
     * @ref getResourceCreate.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setResourceCreate(boolean value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setResourceCreateJNI(value);
    }


    
    /**
     * Native method for @ref getResourceIdleLifespan
     */
    private native int getResourceIdleLifespanJNI();
    /**
     * Gets "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released
     * it; the expiration is necessary to avoid useless resource allocation.
     * The current value can be altered using method
     *     @ref setResourceIdleLifespan.
     * @return the current value
     */
    public int getResourceIdleLifespan() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getResourceIdleLifespanJNI();
    }


    
    /**
     * Native method for @ref setResourceIdleLifespan
     */
    private native int setResourceIdleLifespanJNI(int value);
    /**
     * Sets "resource idle lifespan" property: it specifies how many
     * milliseconds a resource will be kept after the last locker released
     * it; the expiration is necessary to avoid useless resource allocation.
     * The current value can be inspected using method
     *     @ref getResourceIdleLifespan.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setResourceIdleLifespan(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setResourceIdleLifespanJNI(value);
    }


    
    /**
     * Native method for @ref getResourceName
     */
    private native String getResourceNameJNI();
    /**
     * Gets the resource name: the name of the resource that can be locked
     * and unlocked using @ref lock and @ref unlock methods.
     * The current value can be altered using method @ref setResourceName.
     * @return the current value
     */
    public String getResourceName() throws FlomException {
        String ReturnString = null;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            if (null == (ReturnString = getResourceNameJNI()))
                ReturnString = new String("");
        }
        return ReturnString;
    }


    
    /**
     * Native method for @ref setResourceName
     */
    private native int setResourceNameJNI(String value);
    /**
     * Sets the resource name: the name of the resource that can be locked
     * and unlocked using @ref lock and @ref unlock methods.
     * The current value can be inspected using method @ref getResourceName.
     * NOTE: the resource type is determined by its name; take a look to
     * flom command man page (-r, --resource-name option) for an
     * explanation of the resource name grammar.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setResourceName(String value) throws FlomException {
        int ReturnCode = FlomErrorCodes.FLOM_RC_OK;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else if (null == value)
            throw new FlomException(FlomErrorCodes.FLOM_RC_NULL_OBJECT);
        else {
            ReturnCode = setResourceNameJNI(value);
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode &&
                FlomErrorCodes.FLOM_RC_API_IMMUTABLE_HANDLE != ReturnCode)
                throw new FlomException(ReturnCode);
        }
        return ReturnCode;
    }

    
    
    /**
     * Native method for @ref getResourceQuantity
     */
    private native int getResourceQuantityJNI();
    /**
     * Gets "resource quantity" property: the number of units that will be
     * locked and unlocked using @ref lock and @ref unlock methods.
     * The current value can be altered using method
     * @ref setResourceQuantity.
     * NOTE: this property applies to "numeric resources" only.
     * @return the current value
     */
    public int getResourceQuantity() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getResourceQuantityJNI();
    }


    
    /**
     * Native method for @ref setResourceQuantity
     */
    private native int setResourceQuantityJNI(int value);
    /**
     * Sets "resource quantity" property: the number of units that will be
     * locked and unlocked using @ref lock and @ref unlock methods.
     * The current value can be inspected using method
     * @ref getResourceQuantity.
     * NOTE: this property applies to "numeric resources" only.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setResourceQuantity(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setResourceQuantityJNI(value);
    }


    
    /**
     * Native method for @ref getResourceTimeout
     */
    private native int getResourceTimeoutJNI();
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
    public int getResourceTimeout() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return getResourceTimeoutJNI();
    }


    
    /**
     * Native method for @ref setResourceTimeout
     */
    private native int setResourceTimeoutJNI(int value);
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
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setResourceTimeout(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            return setResourceTimeoutJNI(value);
    }


    
    /**
     * Native method for @ref getSocketName
     */
    private native String getSocketNameJNI();
    /**
     * Gets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be altered using method @ref setSocketName.
     * @return the current value as a standard
     */
    public String getSocketName() throws FlomException {
        String ReturnString = null;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            if (null == (ReturnString = getSocketNameJNI()))
                ReturnString = new String("");
        }
        return ReturnString;
    }


    
    /**
     * Native method for @ref setSocketName
     */
    private native int setSocketNameJNI(String value);
    /**
     * Sets the socket name: the AF_LOCAL/AF_UNIX socket name that must be
     * used to contact a local FLoM daemon (server).
     * The current value can be inspected using method @ref getSocketName
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setSocketName(String value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else if (null == value)
            throw new FlomException(FlomErrorCodes.FLOM_RC_NULL_OBJECT);
        else
            return setSocketNameJNI(value);
    }

    
    
    /**
     * Native method for @ref getTraceFilename
     */
    private native String getTraceFilenameJNI();
    /**
     * Gets the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be altered using method @ref setTraceFilename.
     * @return the current value
     */
    public String getTraceFilename() throws FlomException {
        String ReturnString = null;
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            if (null == (ReturnString = getTraceFilenameJNI()))
                ReturnString = new String("");
        }
        return ReturnString;
    }


    
    /**
     * Native method for @ref setTraceFilename
     */
    private native int setTraceFilenameJNI(String value);
    /**
     * Sets the trace filename: the name (absolute or relative path) used
     * by libflom (FLoM client library) to record trace messages.
     * The current value can be inspected using function
     * @ref getTraceFilename.
     * @param value (Input): the new value
     * @return a reason code that can be checked to be sure the property
     *         was changed by the setter method
     */
    public int setTraceFilename(String value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else if (null == value)
            throw new FlomException(FlomErrorCodes.FLOM_RC_NULL_OBJECT);
        else
            return setTraceFilenameJNI(value);
    }

    
    
    // @@@ remove me!!!
    public static void main(String[] args) {
        try {
            FlomHandle fh = new FlomHandle();
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               fh.getDiscoveryAttempts());
            fh.setDiscoveryAttempts(33);
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               fh.getDiscoveryAttempts());
            fh.setDiscoveryTimeout(231);
            System.out.println("FlomHandle.getDiscoveryTimeout() = " +
                               fh.getDiscoveryTimeout());
            fh.setDiscoveryTtl(5);
            System.out.println("FlomHandle.getDiscoveryTtl() = " +
                               fh.getDiscoveryTtl());
            fh.setLockMode(FlomLockModes.FLOM_LOCK_MODE_PR);
            System.out.println("FlomHandle.getLockMode() = " +
                               fh.getLockMode());
            /*
            fh.setMulticastAddress("224.0.0.3");
            try {
                System.out.println("FlomHandle.getMulticastAddress() = '" +
                                   fh.getMulticastAddress() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle: " + e.getMessage());
                else throw(e);
            }
            */
            fh.setMulticastPort(12345);
            System.out.println("FlomHandle.getMulticastPort() = " +
                               fh.getMulticastPort());
           
            fh.setResourceCreate(false);
            System.out.println("FlomHandle.getResourceCreate() = " +
                               fh.getResourceCreate());
            fh.setResourceCreate(true);
            System.out.println("FlomHandle.getResourceCreate() = " +
                               fh.getResourceCreate());
            
            fh.setResourceIdleLifespan(6);
            System.out.println("FlomHandle.getResourceIdleLifespan() = " +
                               fh.getResourceIdleLifespan());

            fh.setResourceQuantity(6);
            System.out.println("FlomHandle.getResourceQuantity() = " +
                               fh.getResourceQuantity());

            fh.setResourceTimeout(69);
            System.out.println("FlomHandle.getResourceTimeout() = " +
                               fh.getResourceTimeout());

            
            // use a wrong resource name
            try {
                fh.setResourceName("an invalid name...");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_INVALID_RESOURCE_NAME ==
                    e.getReturnCode())
                    System.out.println("FlomHandle.setResourceName() EXCP/"
                                       + e.getMessage());
                else throw(e);
            }
            try {
                fh.setResourceName("avalidname");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_INVALID_RESOURCE_NAME ==
                    e.getReturnCode())
                    System.out.println("FlomHandle.setResourceName() EXCP/"
                                       + e.getMessage());
                else throw(e);
            }
            try {
                System.out.println("FlomHandle.getResourceName() = '" +
                                   fh.getResourceName() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle.getResourceName() EXCP/"
                                       + e.getMessage());
                else throw(e);
            }

            
            System.out.println("FlomHandle.getSocketName() = '" +
                               fh.getSocketName() + "'");
            /*
            fh.setSocketName("/tmp/foobar");
            try {
                System.out.println("FlomHandle.getSocketName() = '" +
                                   fh.getSocketName() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle: " + e.getMessage());
                else throw(e);
            }
            fh.setSocketName(null);
            */

            fh.setTraceFilename("/tmp/mytrace");
            try {
                System.out.println("FlomHandle.getTraceFilename() = '" +
                                   fh.getTraceFilename() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle: " + e.getMessage());
                else throw(e);
            }
            fh.setTraceFilename(null);
            
            fh.lock();
            try {
                System.out.println("FlomHandle.getLockedElement() = '" +
                                   fh.getLockedElement() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle.getLockedElement() " +
                                       "EXCP/" + e.getMessage());
                else throw(e);
            }
            fh.unlock();
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() + 
                               " (" + e.getMessage() + ")");
        }
        /*
        System.runFinalization();
        Runtime.getRuntime().runFinalization();
        */
    }
}
