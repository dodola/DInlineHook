package dodola.profiler;

import android.app.Application;
import android.content.Context;



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
        super.attachBaseContext(base);
        try {
            Method onClick = MainActivity.class.getDeclaredMethod("returnString2");
            ArtMethod artOrigin2 = ArtMethod.of(onClick);
            ArtMethod backup = artOrigin2.backup();
            InnerHooker.testMethod(onClick, artOrigin2.getAccessFlags(),backup);

//            artOrigin.ensureResolved();
//            artOrigin.compile();
//            long entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();
//            Profiler.testMethod2(entryPointFromQuickCompiledCode);
//            entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }
    }
}
