//
// Created by didi on 2017/10/30.
//

#ifndef PROFILER_DING_H
#define PROFILER_DING_H


#include <jni.h>
#include <stdlib.h>

extern "C" {
#include <stdint.h>
#include <sys/mman.h>
}

#include <cstdio>
#include <string>
#include "aarch32/constants-aarch32.h"
#include "aarch32/instructions-aarch32.h"
#include "aarch32/macro-assembler-aarch32.h"

using namespace vixl;
using namespace vixl::aarch32;

#include <fb/include/fb/fbjni.h>

using namespace facebook::jni;

extern alias_ref<jclass> nativeEngineClass;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved);


class ExecutableMemory {
public:
    ExecutableMemory(const byte *code_start, size_t size)
            : size_(size),
              buffer_(reinterpret_cast<byte *>(mmap(NULL,
                                                    size,
                                                    PROT_READ | PROT_WRITE | PROT_EXEC,
                                                    MAP_SHARED | MAP_ANONYMOUS,
                                                    -1,
                                                    0))) {
        VIXL_ASSERT(reinterpret_cast<intptr_t>(buffer_) != -1);
        memcpy(buffer_, code_start, size_);
//        __builtin___clear_cache((char *)(buffer_), buffer_ + size_);
    }

    ~ExecutableMemory() { munmap(buffer_, size_); }

    template<typename T>
    T GetEntryPoint(const Label &entry_point, InstructionSet isa) const {
        int32_t location = entry_point.GetLocation();
        if (isa == T32) location += 1;
        return GetOffsetAddress<T>(location);
    }

protected:
    template<typename T>
    T GetOffsetAddress(int32_t offset) const {
        VIXL_ASSERT((offset >= 0) && (static_cast<size_t>(offset) <= size_));
        T function_address;
        byte *buffer_address = buffer_ + offset;
        memcpy(&function_address, &buffer_address, sizeof(T));
        return function_address;
    }

private:
    size_t size_;
    byte *buffer_;
};


#endif //PROFILER_DING_H
