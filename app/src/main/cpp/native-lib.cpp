#include <jni.h>
#include <android/log.h>
#include <stdio.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

void preCall() {
    printf("===============");
}

//extern "C" void addme(int, int);


extern "C"
JNIEXPORT jstring

JNICALL
Java_dodola_profiler_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    LOGW("================");
    return env->NewStringUTF("ddd");
}

jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    printf("cccccccccccccccccccc");
//    addme((size_t) preCall, 2);
}