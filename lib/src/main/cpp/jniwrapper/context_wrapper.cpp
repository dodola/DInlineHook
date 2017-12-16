//
// Created by didi on 2017/11/3.
//

#include <jni.h>
#include "context_wrapper.h"


class CPackageInfo {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

    CPackageInfo(JNIEnv *env, jobject ori, jclass clazz) :
            jniEnv(env),
            originObj(ori),
            clazz(clazz) {

    }

public:
};

class CPackageManager {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

public:
    CPackageManager(JNIEnv *env, jobject ori, jclass clazz) :
            jniEnv(env),
            originObj(ori),
            clazz(clazz) {
    }

    ~CPackageManager() {

    }
};

class CApplicationInfo {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

public:
    CApplicationInfo(JNIEnv *env, jobject ori, jclass clazz) :
            jniEnv(env),
            originObj(ori),
            clazz(clazz) {

    }

};

class CContext {
private:
    JNIEnv *jniEnv;
    jobject originObj;
    jclass clazz;

public:
    CContext(JNIEnv *env, jobject ori, jclass clazz) :
            jniEnv(env),
            originObj(ori),
            clazz(clazz) {

    }

    CPackageManager getPackageManger();

    jstring getPackageName();

    CApplicationInfo getApplicationInfo();

};