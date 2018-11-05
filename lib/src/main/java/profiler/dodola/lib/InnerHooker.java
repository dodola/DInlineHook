package profiler.dodola.lib;

import android.util.Log;

import java.lang.reflect.Method;

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

    public static native void testMethod(Object method, int flags, Object backup);


    public static native long mmap(int length);

    public static native boolean munmap(long address, int length);

    public static void put(byte[] bytes, long dest) {
        memput(bytes, dest);
    }

    public static byte[] get(long src, int length) {
        byte[] bytes = memget(src, length);
        return bytes;
    }


    public static long map(int length) {
        long m = mmap(length);
        return m;
    }

    public static boolean unmap(long address, int length) {
        return munmap(address, length);
    }

    public static void callOrigin(ArtMethod method, Object ori) {
        Log.e("ttttt", "---------------------------" + ori + "," + method + ",");
//        View v = ((View) ori);
//        Toast.makeText(v.getContext(), "什么鬼 ing", Toast.LENGTH_SHORT).show();
        try {
            Object o = method.invokeInternal(ori, null);
            Log.e("ttttt", "getrrrrresult=====" + o);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
