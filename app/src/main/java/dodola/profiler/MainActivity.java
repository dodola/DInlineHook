package dodola.profiler;

import android.app.Activity;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class MainActivity extends Activity implements View.OnClickListener {


    private ImageView iv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //        Profiler.startIOProfiler();

        Button btn = findViewById(R.id.button);
        final TextView tv = findViewById(R.id.sample_text);

//        tv.setText(returnString("ccccx", "vvvvvvc"));
        tv.setText(returnString2());
        iv = findViewById(R.id.imageView);
        //        btn.setText(stringFromJNI());

        btn.setOnClickListener(this);

    }

    public static String returnString(String a, String b) {
        Log.d("dsfsdf", "=================");
        StackTraceElement[] stackTrace = Thread.currentThread().getStackTrace();

        StringBuilder sb = new StringBuilder();
        for (StackTraceElement ele : stackTrace) {
            sb.append(ele.toString());
        }
        return sb.toString();
    }

    public static String returnString2() {
        Log.d("dsfsdf", "=================");
        return "1212121212121";
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    //    public native String stringFromJNI();
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button: {
                //                stringFromJNI();
                Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.mipmap.ic_launcher);
                iv.setImageBitmap(bitmap);
                SharedPreferences llll = getSharedPreferences("llll", MODE_PRIVATE);
                boolean dd = llll.getBoolean("dd", true);
                llll.edit().putBoolean("dd", !dd).commit();

                File file = new File("/sdcard/l.txt");

                try {
                    FileOutputStream outputStream = new FileOutputStream(file);
                    outputStream.write("什么鬼".getBytes());
                    outputStream.close();
                    FileInputStream inputStream = new FileInputStream(file);
                    byte[] bs = new byte[1024];
                    StringBuilder sb = new StringBuilder();
                    while (inputStream.read(bs) != -1) {
                        sb.append(new String(bs));
                    }
                    Log.d("TAGGGG", "sb:" + sb.toString());
                    inputStream.close();
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            break;
        }
    }
}
