package org.tiian.flom;

import java.nio.ByteBuffer;

public class FlomHandle {
    static {
	System.loadLibrary("flom-java");
    }

    private ByteBuffer bb;

    private native ByteBuffer createHandle();

    // @@@ remove me!!!
    private native void sayHello();

    // @@@ remove me!!!
    public static void main(String[] args) {
        FlomHandle fh = new FlomHandle();
        fh.sayHello();
        ByteBuffer prova = fh.createHandle();
    }
}