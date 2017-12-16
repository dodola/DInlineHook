#include <jni.h>
#include <stdio.h>

void preCall() {
    printf("===============");
}

extern "C" void addme(int, int);


jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    printf("cccccccccccccccccccc");
    addme((size_t) preCall, 2);
}