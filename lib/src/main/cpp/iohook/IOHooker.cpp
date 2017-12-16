//
// Created by didi on 2017/10/24.
//

#include <unistd.h>
#include "IOHooker.h"
#include <fcntl.h>
#include <map>
#include <jni.h>
#include <pthread.h>
#include <stdlib.h>
#include <exception>
#include "../jniwrapper/java_lang_wrapper.h"

extern "C" {
#include "../HookZz/include/hookzz.h"
}


//使用HOOKZZ框架
static inline void
hook_function(void *handle, const char *symbol, void *new_func, void **old_func) {
    void *addr = dlsym(handle, symbol);
    if (addr == NULL) {
        return;
    }
    ZzHookReplace(addr, new_func, old_func);
}

std::string Int_to_String(jlong n) {
    std::ostringstream stream;
    stream << n;  //n为int类型
    return stream.str();
}

std::string IOHooker::getStatckString() {
    JNIEnv *env = Environment::current();
    if (env == NULL) {
        return "";
    }
//    jclass profilerClass = env->FindClass("profiler/dodola/lib/Profiler");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return "";
    }


    Thread thread(env, Thread::currentThread(env));
    Throwable throwable = Throwable::constuctor(env);
    jclass steClass = env->FindClass("java/lang/StackTraceElement");

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return "";
    }


    std::string log = "";

    jobjectArray elements = throwable.getStackTrace();
    int len = env->GetArrayLength(elements);

    for (int i = 0; i < len; ++i) {
        jobject ele = env->GetObjectArrayElement(elements, i);
        StackTraceElement e(env, ele, steClass);
        jstring stackStr = e.toString();

        const char *data = env->GetStringUTFChars(stackStr, nullptr);
        log.append(data);
        log.append("|");
        env->ReleaseStringUTFChars(stackStr, data);
        env->DeleteLocalRef(ele);
    }
    env->DeleteLocalRef(steClass);
    env->DeleteLocalRef(elements);


    log.append("ThreadID:");
    log.append(Int_to_String(thread.getId()));
    log.append("|");

    jstring tname = thread.getName();
    const char *data = env->GetStringUTFChars(tname, nullptr);
    log.append(data);
    env->ReleaseStringUTFChars(tname, data);

    return log;
}

std::string filterPaths[20];

void IOHooker::countOpen(const char *pathname, int __fd) {
    std::string strPathName(pathname);
    if (pathname && !pathname[0] && *pathname != '/') {
        return;
    }
    if (strncmp(pathname, "/proc/", 6) == 0) {
        return;
    }
    if (strncmp(pathname, "/dev/", 5) == 0) {
        return;
    }
    if (strncmp(pathname, "/sys/", 5) == 0) {
        return;
    }
    if (strncmp(pathname, "/data/dalvik-cache/", 19) == 0) {
        return;
    }
    if (strncmp(pathname, "/system/", 8) == 0) {
        return;
    }
//    /system/
//    /data/dalvik-cache/
    loge("dodola", "count:%s   %d", pathname, __fd);
    IOHolder *holder = getHolder(strPathName);
    holder->openCount++;
    ioMaps[strPathName] = holder;
    holder->fd = __fd;
    holder->stackTraceLogs.insert(holder->stackTraceLogs.end(), getStatckString());

}

void IOHooker::startProfiler() {
    isStart = true;
}

void IOHooker::stopProfiler(jstring filePath) {
    isStart = false;
    //dump to file
    dumpLogs(filePath);

}

IOHooker::IOHolder *IOHooker::getHolder(std::string pathname) {
    IOHooker::IOHolder *holder;
    if (ioMaps[pathname] == NULL) {
        holder = new IOHolder;
        holder->path = pathname;
        holder->openCount = 0;
    } else {
        holder = ioMaps[pathname];
    }
    //通过Throwable获取线程栈
    return holder;
}

void IOHooker::dumpLogs(jstring filePath) {
    //写入csv
    const char *data = Environment::current()->GetStringUTFChars(filePath, NULL);

    std::string fileStringName = std::string(data);

    DiskFileSystem fs;

    auto file = fs.GetOrNewFile(fileStringName);
    file.get()->OpenForWrite();
    std::vector<std::string>::iterator logIterator;

    std::map<std::string, IOHolder *>::iterator iter;
    for (iter = ioMaps.begin(); iter != ioMaps.end(); iter++) {
        *file << iter->second->path << "," << iter->second->openCount << "," << iter->second->fd
              << ",";
        for (logIterator = iter->second->stackTraceLogs.begin();
             logIterator != iter->second->stackTraceLogs.end(); logIterator++) {
            *file << *logIterator << ",";
        }

        *file << "\n";;

    }
    file.get()->Close();
    Environment::current()->ReleaseStringUTFChars(filePath, data);
}


__BEGIN_DECLS


// int __openat(int fd, const char *pathname, int flags, int mode);
HOOK_DEF(int, __openat, int dirfd, const char *pathname, int flags, int mode) {

    int ret = syscall(__NR_openat, dirfd, pathname, flags, mode);
//    loge("dodola", "open at:%s %d", pathname, ret);
    if (IOHooker::isStart) {
        IOHooker::countOpen(pathname, ret);
    }
    return ret;
}
// int __open(const char *pathname, int flags, int mode);
HOOK_DEF(int, __open, const char *pathname, int flags, int mode) {
//    countOpen(pathname);
    int ret = syscall(__NR_open, pathname, flags, mode);
    if (IOHooker::isStart) {
        IOHooker::countOpen(pathname, ret);
    }
    return ret;
}

//ssize_t read(int __fd, void* __buf, size_t __count)
HOOK_DEF(int, read, int __fd, void *__buf, size_t __count) {
    loge("dodola", "read:%d", __fd);
    int ret = syscall(__NR_read, __fd, __buf, __count);
    return ret;
}
//ssize_t write(int __fd, const void* __buf, size_t __count)
HOOK_DEF(int, write, int __fd, void *__buf, size_t __count) {
    loge("dodola", "write:%d", __fd);
    int ret = syscall(__NR_write, __fd, __buf, __count);
    return ret;
}

//int close(int __fd);
HOOK_DEF(int, close, int __fd) {
    loge("dodola", "close:%d", __fd);
    int ret = syscall(__NR_close, __fd);
    return ret;
}
__END_DECLS

void openat_pre_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {
    loge("dodola", "openat:%s", (char *) rs->general.regs.r0);
//    countOpen((char *) rs->general.regs.r0);

}

void openat_post_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {

}

void read_pre_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {
//    loge("dodola", "read:IIII");
//    countOpen((char *) rs->general.regs.r0);
    loge("hookzz", "precall");

}

void read_post_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {
    loge("hookzz", "postcall");
}

void write_pre_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {

}

void write_post_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack) {

}


void IOHooker::startHook(int api_level, int preview_api_level) {
    void *handle = dlopen("libc.so", RTLD_NOW);

    if (handle) {
        HOOK_SYMBOL(handle, __openat);
//        HOOK_SYMBOL(handle, read);
//        HOOK_SYMBOL(handle, write);
//        HOOK_SYMBOL(handle, close);

        if (api_level <= 20) {
            HOOK_SYMBOL(handle, __open);
        }
        dlclose(handle);
    }

    //
//    if (handle) {
//    ZzHookPrePost((void *) openat, openat_pre_call, openat_post_call);
//    ZzEnableDebugMode();
//    ZzHook((void *) read, NULL, NULL, read_pre_call, read_post_call, FALSE);
//        ZzHookPrePost((void *) read, read_pre_call, read_post_call);
//        ZzHookPrePost((void *) write, write_pre_call, write_post_call);
//    }
}
