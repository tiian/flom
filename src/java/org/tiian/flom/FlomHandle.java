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
     * Call the lock method of native @ref flom_handle_t object
     */
    private native int lockFH();
    /**
     * Call the unlock method of native @ref flom_handle_t object
     */
    private native int unlockFH();
    /**
     * Call the getLockedElement of native @ref flom_handle_t object
     */
    private native String getLockedElementFH();



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
                               " (" + e.getReturnCodeText() + ")");
        }
    }
    
    // @@@ remove me!!!
    public static void main(String[] args) {
        try {
            FlomHandle fh = new FlomHandle();
            fh.lock();
            fh.unlock();
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() + 
                               " (" + e.getReturnCodeText() + ")");
        }
        /*
        System.runFinalization();
        Runtime.getRuntime().runFinalization();
        */
    }
}
