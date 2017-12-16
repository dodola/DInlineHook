////
//// Created by didi on 2017/10/30.
////
//
//#include "exception_wrapper.h"
//#include "dodo_wrapper.h"
//
//using namespace java::lang;
//
//// public java.lang.String java.lang.Throwable.getMessage()
//jstring Throwable::getMessage() const {
//    static jniext::Method<jstring> method(clazz(), "getMessage", "()Ljava/lang/String;");
//    return method.call(*this);
//}
//
//// public void java.lang.Throwable.printStackTrace()
//void Throwable::printStackTrace() const {
//    static jniext::Method<void> method(clazz(), "printStackTrace", "()V");
//    method.call(*this);
//}
//
//const char *Throwable::clazz() {
//    return "java/lang/Throwable";
//}
//
//
//Throwable Throwable::construct() {
//    static jniext::Constructor<Throwable> constructor(clazz(), "()V");
//    return constructor.construct();
//}
//
//
//// public static native java.lang.Thread java.lang.Thread.currentThread()
//Thread Thread::currentThread() {
//    static jniext::StaticMethod<Thread> method(clazz(), "currentThread", "()Ljava/lang/Thread;");
//    return method.call();
//}
//
//jlong Thread::getId() const {
//    static jniext::Method<jlong> method(clazz(), "getId", "()J");
//    return method.call(*this);
//}
//
//// public final java.lang.String java.lang.Thread.getName()
//jstring Thread::getName() const {
//    static jniext::Method<jstring> method(clazz(), "getName", "()Ljava/lang/String;");
//    return method.call(*this);
//}
//
//jobjectArray Throwable::getStackTrace() const {
//    static jniext::Method<jobjectArray> method(clazz(), "getStackTrace",
//                                               "()[Ljava/lang/StackTraceElement;");
//    return method.call(*this);
//}
//
//const char *Thread::clazz() {
//    return "java/lang/Thread";
//}
