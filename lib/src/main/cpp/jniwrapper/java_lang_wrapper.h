//
// Created by didi on 2017/11/3.
//

#ifndef PROFILER_JAVA_LANG_WRAPPER_H
#define PROFILER_JAVA_LANG_WRAPPER_H

#include <jni.h>
#include "../fb/include/fb/fbjni.h"
#include <string>

class StackTraceElement {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

public:
    StackTraceElement(JNIEnv *env, jobject ori, jclass c) :
            jniEnv(env),
            originObj(ori), clazz(c) {
        if (c == NULL) {
            clazz = jniEnv->FindClass("java/lang/StackTraceElement");
        }
    }


    ~StackTraceElement() {
//        jniEnv->DeleteLocalRef(clazz);
        jniEnv = NULL;
    }

    jstring toString() const;
};

class Throwable {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

public:
    Throwable(JNIEnv *env, jobject ori) :
            jniEnv(env),
            originObj(ori) {
        clazz = jniEnv->FindClass("java/lang/Throwable");
    }

    ~Throwable() {
        jniEnv->DeleteLocalRef(clazz);
        jniEnv = NULL;
    }

    static Throwable constuctor(JNIEnv *);

    jstring getMessage() const;

//        public StackTraceElement[] getStackTrace()
    jobjectArray getStackTrace() const;


    void printStackTrace() const;


};

class Thread {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;


public:
    Thread(JNIEnv *env, jobject ori) :
            jniEnv(env),
            originObj(ori) {
        clazz = jniEnv->FindClass("java/lang/Thread");
    }

    static jobject currentThread(JNIEnv *);

    jstring getName() const;

    jlong getId() const;

    ~Thread() {
        jniEnv->DeleteLocalRef(clazz);
        jniEnv = NULL;
    }

};


#endif //PROFILER_JAVA_LANG_WRAPPER_H
