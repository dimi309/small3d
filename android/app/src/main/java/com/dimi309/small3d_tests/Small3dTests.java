package com.dimi309.small3d_tests;

public class Small3dTests extends android.app.NativeActivity {
    static {

        System.loadLibrary("c++_shared");
        System.loadLibrary("oboe");
        System.loadLibrary("small3d_tests");

    }
}
