////
//// Created by didi on 2017/10/30.
////
//
//#ifndef PROFILER_EXCEPTION_WRAPPER_H
//#define PROFILER_EXCEPTION_WRAPPER_H
//#pragma once
//
//#include "dodo_wrapper.h"
//
//namespace java {
//    namespace lang {
//        class Throwable;
//
//        class StackTraceElement;
//
//        class Thread;
//
//        class StackTraceElement : public jniext::Object {
//        public:
//            using jniext::Object::Object;
//        };
//
//        class Throwable : public jniext::Object {
//        public:
//            using jniext::Object::Object;
//
//            jstring getMessage() const;
//
////        public StackTraceElement[] getStackTrace()
//            jobjectArray getStackTrace() const;
//
//            static Throwable construct();
//
//
//            void printStackTrace() const;
//
//            static const char *clazz();
//
//        };
//
//        class Thread : public jniext::Object {
//        public:
//            using jniext::Object::Object;
//
//            static Thread currentThread();
//
//            jstring getName() const;
//
//            jlong getId() const;
//
//            static const char *clazz();
//
//        };
//
//    }
//}
//
//class ProfilerWrapper {
//
//};
//
//#endif //PROFILER_EXCEPTION_WRAPPER_H
