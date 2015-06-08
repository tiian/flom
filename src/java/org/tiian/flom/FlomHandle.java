package org.tiian.flom;

import java.nio.ByteBuffer;

public class FlomHandle {
    static {
	System.loadLibrary("flom-java");
    }

    private ByteBuffer bb;

    /**
     * Create a new native @ref flom_handle_t object and return a pointer
     * to JVM environment
     */
    private native ByteBuffer newFlomHandle();

    private native void deleteFlomHandle(ByteBuffer bb);

    public FlomHandle() {
        bb = newFlomHandle();
    }

    /**
     * Explicitly release the native object allocated by JNI wrapper
     */
    public void release() {
        deleteFlomHandle(bb);
        bb = null;
    }

    /**
     * Release native object if finalization is executed and the program
     * forgot to call @ref release method
     */
    protected void finalize() {
        if (null != bb)
            deleteFlomHandle(bb);
    }
    
    // @@@ remove me!!!
    public static void main(String[] args) {
        FlomHandle fh = new FlomHandle();
        fh.release();
        /*
        System.runFinalization();
        Runtime.getRuntime().runFinalization();
        */
    }
}