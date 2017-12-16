package dodola.profiler;

import java.io.File;
import java.io.FileOutputStream;

import com.taobao.android.dexposed.DexposedBridge;
import com.taobao.android.dexposed.XC_MethodHook;

import android.app.Application;
import android.content.Context;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import me.weishu.epic.art.Epic;
import me.weishu.epic.art.method.ArtMethod;
import me.weishu.epic.art.method.Offset;
import profiler.dodola.lib.Profiler;

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
        new Profiler.Builder(this).setEnableIOTracer(true).setEnableViewServer(true)
            .start();
        try {
            //            ArtMethod artOrigin = ArtMethod.of(MainActivity.class.getDeclaredMethod("returnString", String
            //                    .class,
            //                String.class));
            ArtMethod artOrigin =
                ArtMethod.of(MainActivity.class.getDeclaredMethod("onCreate", Bundle.class));
            ArtMethod artOrigin2 = ArtMethod.of(MainActivity.class.getDeclaredMethod("returnString2"));

            artOrigin.ensureResolved();
            artOrigin.compile();
            long entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();

            Log.e("DDDDD",
                "address:" + entryPointFromQuickCompiledCode);
//            Profiler.testMethod(artOrigin.getAddress(), artOrigin.getAccessFlags());
                        Profiler.testMethod2(entryPointFromQuickCompiledCode);

            entryPointFromQuickCompiledCode = artOrigin.getEntryPointFromQuickCompiledCode();
            Log.e("DDDDD",
                "address:" + entryPointFromQuickCompiledCode);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        }

    }
}
