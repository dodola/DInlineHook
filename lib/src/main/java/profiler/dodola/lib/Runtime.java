package profiler.dodola.lib;

import android.util.Log;

public class Runtime {

    private static final String TAG = "Runtime";

    private volatile static Boolean isThumb = null;

    private volatile static boolean g64 = false;
    private volatile static boolean isArt = true;

    static {
        try {
            g64 = (boolean) Class.forName("dalvik.system.VMRuntime").getDeclaredMethod("is64Bit").invoke(Class.forName("dalvik.system.VMRuntime").getDeclaredMethod("getRuntime").invoke(null));
        } catch (Exception e) {
            Log.e(TAG, "get is64Bit failed, default not 64bit!", e);
            g64 = false;
        }
        isArt = System.getProperty("java.vm.version").startsWith("2");
        Log.i(TAG, "is64Bit: " + g64 + ", isArt: " + isArt);
    }

    public static boolean is64Bit() {
        return g64;
    }

    public static boolean isArt() {
        return isArt;
    }

}