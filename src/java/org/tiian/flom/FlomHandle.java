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
     * Create a new native @ref flom_handle_t object and set @ref NativeHandler
     */
    private native int newFH();
    /**
     * Delete the native @ref flom_handle_t object
     */
    private native int deleteFH();
    /**
     * Call 'lock' method of the native @ref flom_handle_t object
     */
    private native int lockFH();
    /**
     * Call 'unlock' method of the native @ref flom_handle_t object
     */
    private native int unlockFH();
    /**
     * Call 'get_locked_element' method of the native @ref flom_handle_t object
     */
    private native String getLockedElementFH();
    /**
     * Call 'set_discovery_timeout' method of the native
     * @ref flom_handle_t object
     */
    private native void setDiscoveryTimeoutFH(int value);
    /**
     * Call 'set_discovery_ttl' method of the native
     * @ref flom_handle_t object
     */
    private native void setDiscoveryTtlFH(int value);


    
    /**
     * Create a new object calling the native interface
     */
    public FlomHandle() throws FlomException {
        int ReturnCode = newFH();
        if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
            throw new FlomException(ReturnCode);
    }
    /**
     * Explicitly free the native object allocated by JNI wrapper
     */
    public void free() throws FlomException {
        if (null != NativeHandler) {
            int ReturnCode = deleteFH();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
            NativeHandler = null;
        }
    }


    
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
            int ReturnCode = lockFH();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
        }
    }



    /**
     * Unlock the (logical) resource linked to this handle; the resource
     * MUST be previously locked using method @ref lock
     */
    public void unlock() throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else {
            int ReturnCode = unlockFH();
            if (FlomErrorCodes.FLOM_RC_OK != ReturnCode)
                throw new FlomException(ReturnCode);
        }
    }


    
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
            if (null == (ReturnString = getLockedElementFH()))
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
    private native void setDiscoveryAttemptsJNI(int value);
    /**
     * Sets the maximum number of attempts that will be tryed during
     * auto-discovery phase using UDP/IP multicast (see
     *         @ref setMulticastAddress, @ref setMulticastPort).
     * The current value can be inspected using method
     *         @ref getDiscoveryAttempts
     * @param value (Input): the new value
     */
    public void setDiscoveryAttempts(int value) throws FlomException {
        if (null == NativeHandler)
            throw new FlomException(FlomErrorCodes.FLOM_RC_OBJ_CORRUPTED);
        else
            setDiscoveryAttemptsJNI(value);
    }

    
    
    /**
     * Gets the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * @ref getMulticastAddress, @ref getMulticastPort).
     * The current value can be altered using method
     * @ref setDiscoveryTimeout
     * @return the current value
     */
    public native int getDiscoveryTimeout();


    
    /**
     * Sets the number of milliseconds between two consecutive attempts that
     * will be tryed during auto-discovery phase using UDP/IP multicast (see
     * @ref setMulticastAddress, @ref setMulticastPort).
     * The current value can be inspected using method
     * @ref getDiscoveryTtl.
     * @param value (Input): the new value
     */
    public void setDiscoveryTimeout(int value) {
        setDiscoveryTimeoutFH(value);
    }

    
    
    /**
     * Gets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be altered using method @ref setDiscoveryTtl
     * @return the current value
     */
    public native int getDiscoveryTtl();

    
    /**
     * Sets the UDP/IP multicast TTL parameter used during auto-discovery
     * phase; for a definition of the parameter, see
     * http://www.tldp.org/HOWTO/Multicast-HOWTO-2.html
     * . The current value can be inspected using method
     * @ref getDiscoveryTtl.
     * @param value (Input): the new value
     */
    public void setDiscoveryTtl(int value) {
        setDiscoveryTtlFH(value);
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
    
    // @@@ remove me!!!
    public static void main(String[] args) {
        try {
            FlomHandle fh = new FlomHandle();
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               fh.getDiscoveryAttempts());
            fh.lock();
            try {
                System.out.println("FlomHandle.getLockedElement() = '" +
                                   fh.getLockedElement() + "'");
            } catch(FlomException e) {
                if (FlomErrorCodes.FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE ==
                    e.getReturnCode())
                    System.out.println("FlomHandle: " + e.getMessage());
                else throw(e);
            }
            fh.setDiscoveryAttempts(33);
            System.out.println("FlomHandle.getDiscoveryAttempts() = " +
                               fh.getDiscoveryAttempts());
            fh.setDiscoveryTimeout(231);
            System.out.println("FlomHandle.getDiscoveryTimeout() = " +
                               fh.getDiscoveryTimeout());
            fh.setDiscoveryTtl(5);
            System.out.println("FlomHandle.getDiscoveryTtl() = " +
                               fh.getDiscoveryTtl());

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
