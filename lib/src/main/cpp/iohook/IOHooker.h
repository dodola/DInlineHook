//
// VirtualApp Native Project
//

#ifndef NDK_HOOK_H
#define NDK_HOOK_H


#include <string>
#include <map>
#include <list>
#include <jni.h>
#include <dlfcn.h>
#include <stddef.h>
#include <fcntl.h>
#include<dirent.h>
#include <sys/syscall.h>

#include "utils/fs/file_system.h"
#include <fb/include/fb/ALog.h>
#include <fb/include/fb/fbjni.h>
#include <utils/fs/disk_file_system.h>
#include <vector>

using namespace facebook::jni;
using namespace facebook::alog;
using namespace profiler;

//#define HOOK_SYMBOL(handle, func) hook_template(handle, #func, (void*) new_##func, (void**) &orig_##func)
#define HOOK_SYMBOL(handle, func) hook_function(handle, #func, (void*) new_##func, (void**) &orig_##func)
#define HOOK_DEF(ret, func, ...) \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)


namespace IOHooker {
    static bool isStart = false;

    void startHook(int api_level, int preview_api_level);

    void startProfiler();

    void stopProfiler(jstring filePath);

    struct IOHolder {
        std::string path;//文件路径
        int fd;
        int openCount;//打开次数
        std::vector<std::string> stackTraceLogs;                                //声明一个int型向量a
    };


    static std::map<std::string, IOHolder *> ioMaps;


    void countOpen(const char *pathname, int __fd);

    void dumpLogs(jstring filePath);

    IOHolder *getHolder(std::string);

    std::string getStatckString();

}

#endif //NDK_HOOK_H
