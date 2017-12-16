package profiler.dodola.lib;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;

import com.android.debug.hv.ViewServer;

/**
 * @author dodola
 * @date 2017/10/23
 */

public class Profiler {
    static {

        try {
            System.loadLibrary("dodo");
        } catch (Throwable e) {
        }

    }

    private static int getPreviewSDKInt() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            try {
                return Build.VERSION.PREVIEW_SDK_INT;
            } catch (Throwable e) {
                // ignore
            }
        }
        return 0;
    }

    private static void enableViewServer(Application application) {
        application.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
                ViewServer.get(activity).addWindow(activity);
            }

            @Override
            public void onActivityStarted(Activity activity) {

            }

            @Override
            public void onActivityResumed(Activity activity) {
                ViewServer.get(activity).setFocusedWindow(activity);
            }

            @Override
            public void onActivityPaused(Activity activity) {

            }

            @Override
            public void onActivityStopped(Activity activity) {

            }

            @Override
            public void onActivitySaveInstanceState(Activity activity, Bundle outState) {

            }

            @Override
            public void onActivityDestroyed(Activity activity) {
                ViewServer.get(activity).removeWindow(activity);
            }
        });
    }

    private static native void nativeEnableIOProfiler(int apiLevel, int previewApiLevel);

    public static native void startIOProfiler();

    public static native void stopIOProfiler(String filePath);

    public static native void testMethod(long address, int replace);

    public static native long getJniOffset();

    public static native long getHookMethodAddress();

    public static native void testMethod2(long entryPointFromQuickCompiledCode);

    public static class Builder {
        private Application application;

        private boolean enableIOTracer;
        private boolean enableViewServer;
        private boolean enableFridaServer;

        public Builder(Application _context) {
            application = _context;
        }

        public Builder setEnableIOTracer(boolean enableIOTracer) {
            this.enableIOTracer = enableIOTracer;
            return this;
        }

        public Builder setEnableViewServer(boolean enableViewServer) {
            this.enableViewServer = enableViewServer;
            return this;
        }

        public Builder setEnableFridaServer(boolean enableFridaServer) {
            this.enableFridaServer = enableFridaServer;
            return this;
        }

        public void start() {
            if (enableViewServer) {
                Profiler.enableViewServer(application);
            }
            if (enableIOTracer) {
                Profiler.nativeEnableIOProfiler(Build.VERSION.SDK_INT, getPreviewSDKInt());
            }
            if (enableFridaServer) {
                //                new Thread(new Runnable() {
                //                    @Override
                //                    public void run() {
                try {
                    System.loadLibrary("frida-gadget");
                } catch (Throwable e) {
                    e.printStackTrace();
                }
                //                    }
                //                });
            }
        }
    }
}
