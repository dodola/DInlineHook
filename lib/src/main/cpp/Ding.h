//
// Created by didi on 2017/10/30.
//

#ifndef PROFILER_DING_H
#define PROFILER_DING_H


#include <jni.h>
#include <stdlib.h>

#include <fb/include/fb/fbjni.h>
#include <IOHooker.h>

using namespace facebook::jni;

extern alias_ref<jclass> nativeEngineClass;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved);


#endif //PROFILER_DING_H
