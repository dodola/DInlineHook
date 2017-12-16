//
// Created by didi on 2017/11/3.
//

#include "java_lang_wrapper.h"

jstring Throwable::getMessage() const {
    return nullptr;
}

jobjectArray Throwable::getStackTrace() const {
    jmethodID mid = jniEnv->GetMethodID(clazz, "getStackTrace",
                                        "()[Ljava/lang/StackTraceElement;");
    return (jobjectArray) jniEnv->CallObjectMethod(originObj, mid);
}

void Throwable::printStackTrace() const {

}

Throwable Throwable::constuctor(JNIEnv *env) {
    jclass clazz = env->FindClass("java/lang/Throwable");
    jmethodID mid = env->GetMethodID(clazz, "<init>", "()V");
    jobject newT = env->NewObject(clazz, mid);
    return Throwable(env, newT);
}

jobject Thread::currentThread(JNIEnv *env) {
    jclass clazz = env->FindClass("java/lang/Thread");
    jmethodID currentThreadMethodId = env->GetStaticMethodID(clazz, "currentThread",
                                                             "()Ljava/lang/Thread;");
    return env->CallStaticObjectMethod(clazz, currentThreadMethodId);
}

jstring Thread::getName() const {
    jmethodID getNameMethodId = jniEnv->GetMethodID(clazz, "getName", "()Ljava/lang/String;");
    return (jstring) jniEnv->CallObjectMethod(originObj, getNameMethodId);
}

jlong Thread::getId() const {
    jmethodID getIdMethodId = jniEnv->GetMethodID(clazz, "getId", "()J");
    return jniEnv->CallLongMethod(originObj, getIdMethodId);
}


jstring StackTraceElement::toString() const {
    jmethodID toStringMethodId = jniEnv->GetMethodID(clazz, "toString", "()Ljava/lang/String;");
    return (jstring) jniEnv->CallObjectMethod(originObj, toStringMethodId);
}
