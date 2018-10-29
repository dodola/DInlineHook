package dodola.profiler;

import android.app.Application;
import android.content.Context;
import android.view.View;

import java.lang.reflect.Method;

import profiler.dodola.lib.ArtMethod;
import profiler.dodola.lib.InnerHooker;

/**
 * Created by didi on 2017/10/24.
 */

public class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    protected void attachBaseContext(Context base) {
//        Debug.waitForDebugger();
        super.attachBaseContext(base);
        try {
//            ArtMethod artOrigin =
//                    ArtMethod.of(MainActivity.class.getDeclaredMethod("onCreate", Bundle.class));
            Method onClick = MainActivity.class.getDeclaredMethod("onClick", View.class);
            ArtMethod artOrigin2 = ArtMethod.of(onClick);
            InnerHooker.testMethod(onClick, artOrigin2.getAccessFlags());

//            artOrigin.ensureResolved();
//            artOrigin.compile();
//            long entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();
//
//            Profiler.testMethod2(entryPointFromQuickCompiledCode);
//
//            entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }
}
