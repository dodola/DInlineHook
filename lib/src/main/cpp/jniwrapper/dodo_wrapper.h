////
//// Created by didi on 2017/10/30.
////
//
//#ifndef PROFILER_DODO_WRAPPER_H
//#define PROFILER_DODO_WRAPPER_H
//
//#include <jni.h>
//#include "../fb/include/fb/fbjni.h"
//#include <string>
//
//using namespace facebook::jni;
//namespace jniext {
//
//
//    class Class;
//
//
//    template<typename T>
//    class Array;
//
//
//#define METHOD_FOR_ALL_TYPE \
//    M(jboolean, Boolean) \
//    M(jchar, Char) \
//    M(jbyte, Byte) \
//    M(jshort, Short) \
//    M(jint, Int) \
//    M(jlong, Long) \
//    M(jfloat, Float) \
//    M(jdouble, Double)
//
//
//    class Object {
//    protected:
//        jobject ori_value;
//
//        JNIEnv *env() const {
//            return Environment::current();
//        }
//
//
//    public:
//        Object(jobject value) : ori_value(value) {
//        }
//
//        operator jobject() const {
//            return ori_value;
//        }
//
//        jstring javaToString() const;
//
//        Class getClass() const;
//
//        static const char *clazz() const;
//    };
//
//
//    class Class : public Object {
//    public:
//        using Object::Object;
//
//        static Class forName(const char *name);
//
//        operator jclass() const {
//            return (jclass) (jobject) *this;
//        }
//
//        jstring getName() const;
//
//        Class getSuperclass() const;
//
//        static const char *clazz();
//
//    };
//
//
//    template<typename T>
//    class Array : public Object {
//    public:
//        using Object::Object;
//
//        operator jarray() const {
//            return (jarray) (jobject) *this;
//        }
//
//        operator jobjectArray() const {
//            return (jobjectArray) (jobject) *this;
//        }
//
//        jsize length() const {
//            return env()->GetArrayLength((jarray) *this);
//        }
//
//        T get(jsize index) const {
//            return env()->GetObjectArrayElement((jobjectArray) *this, index);
//        }
//
//        void set(jsize index, T value) {
//            env()->SetObjectArrayElement((jobjectArray) *this, index, value);
//        }
//
//        const T operator[](jsize index) const {
//            return get(index);
//        }
//
//        static Array<T> create(jsize length) {
//            jclass elementClass = (jclass) T::clazz();
//            return Environment::current()->NewObjectArray(length, elementClass, nullptr);
//        }
//    };
//
////////////////////////////method field/////////////////////////////////
////////////////////////////构造方法/////////////////////////////////
//
//
//    template<typename R, typename... A>
//    class Constructor {
//    protected:
//        jclass current_class;
//        const char *_clsName;
//        const char *_signature;
//        mutable jmethodID _methodID;
//    public:
//        Constructor(const char *clsName, const char *signature) : current_class(nullptr),
//                                                                  _clsName(clsName),
//                                                                  _signature(signature),
//                                                                  _methodID(0) {
//        }
//
//        jmethodID getMethodID() const {
//            if (_methodID == nullptr) {
//                JNIEnv *env = Environment::current();
//                jclass cls = getClass();
//                jmethodID res = env->GetMethodID(cls, "<init>", _signature);
//                _methodID = res;
//            }
//            return _methodID;
//        }
//
//        jclass getClass() const {
//            JNIEnv *env = Environment::current();
//            jclass cls = current_class ? current_class : env->FindClass(_clsName);
//            return cls;
//        }
//
//        operator jmethodID() const {
//            return getMethodID();
//        }
//
//        operator jclass() const {
//            return getClass();
//        }
//
//        R construct(A... args) const {
//            return
//                    Environment::current()->NewObject(getClass(), getMethodID(),
//                                                      convertArg(args)...);
//        }
//
//        R operator()(A... args) const {
//            return construct(args...);
//        }
//    };
//
//    class MethodBase {
//    protected:
//        jclass current_class;
//        const char *clsName;
//        const char *method_name;
//        const char *method_signature;
//        mutable jmethodID _methodID;
//    public:
//        MethodBase(const char *clsName, const char *name, const char *signature) : current_class(
//                nullptr),
//                                                                                   clsName(
//                                                                                           clsName),
//                                                                                   method_name(
//                                                                                           name),
//                                                                                   method_signature(
//                                                                                           signature),
//                                                                                   _methodID(0) {
//        }
//
//        MethodBase(Class &cls, const char *name, const char *signature) : current_class(
//                (jclass) (jobject) cls), clsName(nullptr), method_name(name), method_signature(
//                signature),
//                                                                          _methodID(0) {
//        }
//
//        jmethodID getMethodID() const {
//            if (_methodID == nullptr) {
//                JNIEnv *env = Environment::current();
//                jclass cls = current_class ? current_class : env->FindClass(clsName);
//                jmethodID res = env->GetMethodID(cls, method_name, method_signature);
//                _methodID = res;
//            }
//            return _methodID;
//        }
//
//        operator jmethodID() const {
//            return getMethodID();
//        }
//    };
//
//
//#define M(type, tag) \
//inline type convertArg(type value) { \
//    return value; \
//}
//
//    METHOD_FOR_ALL_TYPE
//
//#undef M
//
//
//    inline jstring convertArg(jstring value) {
//        return value;
//    }
//
//    inline jobject convertArg(jobject value) {
//        return value;
//    }
//
//    inline jclass convertArg(jclass value) {
//        return value;
//    }
//
//    template<typename S>
//    inline jobject convertArg(const S &value) {
//        return (jobject) value;
//    }
//
//
//    template<typename R, typename... A>
//    class Method : public MethodBase {
//    public:
//        using MethodBase::MethodBase;
//
//        R call(jobject target, A... args) const {
//            return (
//                    Environment::current()->CallObjectMethod(target, getMethodID(),
//                                                             convertArg(args)...));
//        }
//
//        R operator()(jobject target, A... args) const {
//            return call(target, args...);
//        }
//    };
//
//    template<typename... A>
//    class Method<void, A...> : public MethodBase {
//    public:
//        using MethodBase::MethodBase;
//
//        void call(jobject target, A... args) const {
//            Environment::current()->CallVoidMethod(target, getMethodID(), convertArg(args)...);
//        }
//
//        void operator()(jobject target, A... args) const {
//            call(target, args...);
//        }
//    };
//
//    template<typename... A>
//    class Method<jstring, A...> : public MethodBase {
//    public:
//        using MethodBase::MethodBase;
//
//        jstring call(jobject target, A... args) const {
//            return (jstring) Environment::current()->CallObjectMethod(target, getMethodID(),
//                                                                      convertArg(args)...);
//        }
//
//        jstring operator()(jobject target, A... args) const {
//            return call(target, args...);
//        }
//    };
//
//#define M(type, tag) \
//template<typename... A> \
//class Method<type, A...> : public MethodBase \
//{ \
//public: \
//    using MethodBase::MethodBase; \
//    type call(jobject target, A... args) const { \
//        return Environment::current()->Call ## tag ## Method(target, getMethodID(), convertArg(args)...); \
//    } \
//    type operator()(jobject target,  A... args) const { \
//        return call(target, args...); \
//    } \
//};
//
//    METHOD_FOR_ALL_TYPE
//
//#undef M
//
//
//    class StaticMethodBase {
//    protected:
//        jclass _cls;
//        const char *_clsName;
//        const char *_name;
//        const char *_signature;
//        mutable jmethodID _methodID;
//    public:
//        StaticMethodBase(const char *clsName, const char *name, const char *signature) : _cls(
//                nullptr), _clsName(clsName), _name(name), _signature(signature), _methodID(0) {
//        }
//
//
//        jmethodID getMethodID() const {
//            if (_methodID == nullptr) {
//
//                JNIEnv *env = Environment::current();
//                jclass cls = getClass();
//                jmethodID res = env->GetStaticMethodID(cls, _name, _signature);
//                _methodID = res;
//            }
//            return _methodID;
//        }
//
//        jclass getClass() const {
//            JNIEnv *env = Environment::current();
//            jclass cls = _cls ? _cls : env->FindClass(_clsName);
//            return cls;
//        }
//
//        operator jmethodID() const {
//            return getMethodID();
//        }
//
//        operator jclass() const {
//            return getClass();
//        }
//    };
//
//    template<typename R, typename... A>
//    class StaticMethod : public StaticMethodBase {
//    public:
//        using StaticMethodBase::StaticMethodBase;
//
//        R call(A... args) const {
//            return
//                    Environment::current()->CallStaticObjectMethod(getClass(), getMethodID(),
//                                                                   convertArg(args)...);
//        }
//
//        R operator()(A... args) const {
//            return call(args...);
//        }
//    };
//
//    template<typename... A>
//    class StaticMethod<void, A...> : public StaticMethodBase {
//    public:
//        using StaticMethodBase::StaticMethodBase;
//
//        void call(A... args) const {
//            return Environment::current()->CallStaticVoidMethod(getClass(), getMethodID(),
//                                                                convertArg(args)...);
//        }
//
//        void operator()(A... args) const {
//            call(args...);
//        }
//    };
//
//
//#define M(type, tag) \
//template<typename... A> \
//class StaticMethod<type, A...> : public StaticMethodBase \
//{ \
//public: \
//    using StaticMethodBase::StaticMethodBase; \
//    type call( A... args) const { \
//        return Environment::current()->CallStatic ## tag ## Method(getClass(), getMethodID(), convertArg(args)...); \
//    } \
//    type operator()( A... args) const { \
//        return call(args...); \
//    } \
//};
//
//    METHOD_FOR_ALL_TYPE
//
//#undef M
//
//
//////////////////////////////////实现/////////////////////////////////////
//
//    inline jstring Object::javaToString() const {
//        Method <jstring> method("java/lang/Object", "toString", "()Ljava/lang/String;");
//        return method.call(*this);
//    }
//
//    inline Class Object::getClass() const {
//        return env()->GetObjectClass((jobject) *this);
//    }
//
//    inline const char *Object::clazz() const {
//        return "java/lang/Object";
//    }
//
//
//    inline Class Class::forName(const char *name) {
//        return (jobject) Environment::current()->FindClass(name);
//    }
//
//    inline jstring Class::getName() const {
//        static Method <jstring> method("java/lang/Class", "getName", "()Ljava/lang/String;");
//        return method.call(*this);
//    }
//
//    inline Class Class::getSuperclass() const {
//        return (env()->GetSuperclass((jclass) *this));
//    }
//
//    inline const char *Class::clazz() {
//        return "java/lang/Class";
//    }
//}
//#endif //PROFILER_DODO_WRAPPER_H
