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
     * Create a new native @ref flom_handle_t object and return a pointer
     * to JVM environment
     */
    private native ByteBuffer newFlomHandle();

    private native void deleteFlomHandle(ByteBuffer NativeHandler);

    public FlomHandle() {
        NativeHandler = newFlomHandle();
    }

    /**
     * Explicitly free the native object allocated by JNI wrapper
     */
    public void free() throws FlomException {
        if (null != NativeHandler) {
            deleteFlomHandle(NativeHandler);
            NativeHandler = null;
        } else {
            /* @@@ use FLOM constant here! SWIG...!!! */
            FlomException e = new FlomException(-4);
            throw e;
        }
    }

    /**
     * Release native object if finalization is executed and the program
     * forgot to call @ref release method
     */
    protected void finalize() {
        try {
            if (null != NativeHandler)
                free();
        } catch(Exception e) {
        }
    }
    
    // @@@ remove me!!!
    public static void main(String[] args) {
        FlomHandle fh = new FlomHandle();
        try {
            fh.free();
        } catch(FlomException e) {
            System.out.println("FlomException: ReturnCode=" +
                               e.getReturnCode() + 
                               " (" + e.getReturnCodeText() + ")");

        }
        FlomException excp = new FlomException(-9);
        System.out.println("ReturnCode=" + excp.getReturnCode() + 
                           " (" + excp.getReturnCodeText() + ")");
        /*
        System.runFinalization();
        Runtime.getRuntime().runFinalization();
        */
    }
}
