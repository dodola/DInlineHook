package profiler.dodola.lib;

import java.lang.reflect.Member;

/**
 * Created by dodola on 2018/10/22.
 */
public class InnerHooker {
    private InnerHooker() {
    }

    static {
        System.loadLibrary("dodo");
    }

    public static native void memput(byte[] bytes, long dest);

    public static native byte[] memget(long src, int length);

    public static native long getMethodAddress(Object method);

    public static native void testMethod(Object method, int flags);

    public static void put(byte[] bytes, long dest) {
        memput(bytes, dest);
    }

    public static byte[] get(long src, int length) {
        byte[] bytes = memget(src, length);
        return bytes;
    }
}
