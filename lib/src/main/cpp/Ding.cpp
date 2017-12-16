//
// Created by didi on 2017/10/30.
//

#include "Ding.h"
#include <fb/Build.h>
#include <fb/ALog.h>
#include <fb/fbjni.h>
#include <unistd.h>
#include <stdarg.h>
#include <HookZz/include/hookzz.h>
#include <sys/mman.h>


using namespace facebook::jni;
using namespace facebook::alog;
//
#define G_GINT64_MODIFIER "l"

#define GSIZE_TO_POINTER(s)    ((gpointer) (gsize) (s))
#define GPOINTER_TO_SIZE(p)    ((gsize) (p))

typedef signed long gint64;
typedef unsigned long guint64;
typedef unsigned int gsize;
typedef void *gpointer;
typedef guint64 GumAddress;
enum Register {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
    TR = 9,  // thread register
    FP = 11,
    IP = 12,
    SP = 13,
    LR = 14,
    PC = 15,
    kNumberOfCoreRegisters = 16,
    kNoRegister = -1,
};
enum {
    H = 1 << 5,
    L = 1 << 20,
    S = 1 << 20,
    W = 1 << 21,
    A = 1 << 21,
    B = 1 << 22,
    N = 1 << 22,
    U = 1 << 23,
    P = 1 << 24,
    I = 1 << 25,

    B0 = 1,
    B1 = 1 << 1,
    B2 = 1 << 2,
    B3 = 1 << 3,
    B4 = 1 << 4,
    B5 = 1 << 5,
    B6 = 1 << 6,
    B7 = 1 << 7,
    B8 = 1 << 8,
    B9 = 1 << 9,
    B10 = 1 << 10,
    B11 = 1 << 11,
    B12 = 1 << 12,
    B13 = 1 << 13,
    B14 = 1 << 14,
    B15 = 1 << 15,
    B16 = 1 << 16,
    B17 = 1 << 17,
    B18 = 1 << 18,
    B19 = 1 << 19,
    B20 = 1 << 20,
    B21 = 1 << 21,
    B22 = 1 << 22,
    B23 = 1 << 23,
    B24 = 1 << 24,
    B25 = 1 << 25,
    B26 = 1 << 26,
    B27 = 1 << 27,
    B28 = 1 << 28,
    B29 = 1 << 29,
    B30 = 1 << 30,
    B31 = 1 << 31,

    RdMask = 15 << 12,
    CondMask = 15 << 28,
    CoprocessorMask = 15 << 8,
    OpCodeMask = 15 << 21,
    Imm24Mask = (1 << 24) - 1,
    Off12Mask = (1 << 12) - 1,

    kLdExRnShift = 16,
    kLdExRtShift = 12,
    kStrExRnShift = 16,
    kStrExRdShift = 12,
    kStrExRtShift = 0,
};

int kAccPublic = 0x0001;
int kAccStatic = 0x0008;
int kAccFinal = 0x0010;
int kAccNative = 0x0100;
int kAccFastNative = 0x00080000;

extern "C" jobject hookMe(JNIEnv *env, jobject objOrClass, ...);

extern "C" {
#include "HookZz/include/hookzz.h"
}


alias_ref<jclass> nativeEngineClass;

static void jni_nativeEnableIOProfiler(alias_ref<jclass>, jint apiLevel,
                                       jint preview_api_level) {
    IOHooker::startHook(apiLevel, preview_api_level);
}

static void jni_startIOProfiler(alias_ref<jclass>) {
    IOHooker::startProfiler();
}

static void jni_stopIOProfiler(alias_ref<jclass>, jstring filePath) {
    IOHooker::stopProfiler(filePath);
}

void hook_post_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack, const HookEntryInfo *info) {
    loge("hookzz", "=========postcall============%x", info->hook_address);
}

void hook_pre_call(RegState *rs, ThreadStack *threadstack, CallStack *callstack, const HookEntryInfo *info) {
    loge("hookzz", "=========hook_pre_call============%s", rs->general.regs.r0);

}


struct _GumMemoryRange {
    GumAddress base_address;
    gsize size;
};
typedef struct _GumMemoryRange GumMemoryRange;


struct _GumRuntimeBounds {
    gpointer start;
    gpointer end;
};

typedef struct _GumRuntimeBounds GumRuntimeBounds;


struct JavaVMExt : public JavaVM {
public:
    void *getRuntime() {
        return runtime_;
    }

private:
    void *const runtime_;
};


struct ArtMethodSpec {
    size_t size;
    size_t jniCode;
    size_t quickCode;
    size_t accessFlags;
    size_t interpreterCode;
};

struct RuntimeOffset {
    size_t heapOffset;
    size_t threadList;
    size_t internTable;
    size_t classLinker;
};

struct ClassLinkerOffset {
    size_t quick_resolution_trampoline_;
    size_t quick_generic_jni_trampoline_;
    size_t quick_to_interpreter_bridge_trampoline_;
};

/*
 * On Android 5.x:
 *
 * class ClassLinker {
 * ...
 * InternTable* intern_table_;                          <-- We find this then calculate our way forwards
 * const void* portable_resolution_trampoline_;
 * const void* quick_resolution_trampoline_;
 * const void* portable_imt_conflict_trampoline_;
 * const void* quick_imt_conflict_trampoline_;
 * const void* quick_generic_jni_trampoline_;           <-- ...to this
 * const void* quick_to_interpreter_bridge_trampoline_;
 * ...
 * }
 *
 * On Android 6.x and above:
 *
 * class ClassLinker {
 * ...
 * InternTable* intern_table_;                          <-- We find this then calculate our way forwards
 * const void* quick_resolution_trampoline_;
 * const void* quick_imt_conflict_trampoline_;
 * const void* quick_generic_jni_trampoline_;           <-- ...to this
 * const void* quick_to_interpreter_bridge_trampoline_;
 * ...
 * }
 */

/*
 * class Runtime {
 * ...
 * gc::Heap* heap_;                <-- we need to find this
 * std::unique_ptr<ArenaPool> jit_arena_pool_;     <----- API level >= 24
 * std::unique_ptr<ArenaPool> arena_pool_;             __
 * std::unique_ptr<ArenaPool> low_4gb_arena_pool_; <--|__ API level >= 23
 * std::unique_ptr<LinearAlloc> linear_alloc_;         \_
 * size_t max_spins_before_thin_lock_inflation_;
 * MonitorList* monitor_list_;
 * MonitorPool* monitor_pool_;
 * ThreadList* thread_list_;        <--- and these
 * InternTable* intern_table_;      <--/
 * ClassLinker* class_linker_;      <-/
 * SignalCatcher* signal_catcher_;
 * std::string stack_trace_file_;
 * JavaVMExt* java_vm_;             <-- so we find this then calculate our way backwards
 * ...
 * }
 */


uint32_t movw(Register rd, uint16_t imm16) {
    uint32_t imm4 = (imm16 >> 12) & 15U /* 0b1111 */;
    uint32_t i = (imm16 >> 11) & 1U /* 0b1 */;
    uint32_t imm3 = (imm16 >> 8) & 7U /* 0b111 */;
    uint32_t imm8 = imm16 & 0xff;
    uint32_t encoding = B31 | B30 | B29 | B28 |
                        B25 | B22 |
                        static_cast<uint32_t>(rd) << 8 |
                        i << 26 |
                        imm4 << 16 |
                        imm3 << 12 |
                        imm8;
    return encoding;
}

uint32_t movt(Register rd, uint16_t imm16) {
    uint32_t imm4 = (imm16 >> 12) & 15U /* 0b1111 */;
    uint32_t i = (imm16 >> 11) & 1U /* 0b1 */;
    uint32_t imm3 = (imm16 >> 8) & 7U /* 0b111 */;
    uint32_t imm8 = imm16 & 0xff;
    uint32_t encoding = B31 | B30 | B29 | B28 |
                        B25 | B23 | B22 |
                        static_cast<uint32_t>(rd) << 8 |
                        i << 26 |
                        imm4 << 16 |
                        imm3 << 12 |
                        imm8;
    return encoding;
}

inline int16_t ldr(Register rt, int32_t offset) {
    return B14 | B11 | (static_cast<int32_t>(rt) << 8) | (offset >> 2);
}

inline int16_t blx(Register rm) {
    int16_t encoding = B14 | B10 | B9 | B8 | B7 | static_cast<int16_t>(rm) << 3;
    return encoding;
}

static int32_t arm_thumb_blx(int32_t immediate) {
    // The value doesn't encode the low two bits (always zero) and is offset by
    // four (see fixup_arm_thumb_cp). The 32-bit immediate value is encoded as
    //   imm32 = SignExtend(S:I1:I2:imm10H:imm10L:00)
    // where I1 = NOT(J1 ^ S) and I2 = NOT(J2 ^ S).
    // The value is encoded into disjoint bit positions in the destination
    // opcode. x = unchanged, I = immediate value bit, S = sign extension bit,
    // J = either J1 or J2 bit, 0 = zero.
    //
    //   BLX: xxxxxSIIIIIIIIII xxJxJIIIIIIIIII0
    //
    // Note that the halfwords are stored high first, low second; so we need
    // to transpose the fixup value here to map properly.
    uint32_t offset = (immediate - 2) >> 2;
    uint32_t signBit = (offset & 0x400000) >> 22;
    uint32_t I1Bit = (offset & 0x200000) >> 21;
    uint32_t J1Bit = (I1Bit ^ 0x1) ^signBit;
    uint32_t I2Bit = (offset & 0x100000) >> 20;
    uint32_t J2Bit = (I2Bit ^ 0x1) ^signBit;
    uint32_t imm10HBits = (offset & 0xFFC00) >> 10;
    uint32_t imm10LBits = (offset & 0x3FF);

    uint32_t Binary = 0;
    uint32_t firstHalf = (((uint16_t) signBit << 10) | (uint16_t) imm10HBits);
    uint32_t secondHalf = (((uint16_t) J1Bit << 13) | ((uint16_t) J2Bit << 11) | ((uint16_t) imm10LBits) << 1);
    Binary |= secondHalf;
    Binary |= firstHalf << 16;
    Binary |= 0xF000C000;

    uint32_t Byte0 = (Binary & 0xFF000000) >> 8;
    uint32_t Byte1 = (Binary & 0x00FF0000) << 8;
    uint32_t Byte2 = (Binary & 0x0000FF00) >> 8;
    uint32_t Byte3 = (Binary & 0x000000FF) << 8;

    Binary = Byte0 | Byte1 | Byte2 | Byte3;
    return Binary;
}

/**
 * 错误的算法
 * @param dstAddr
 * @param srcAddr
 * @return
 */
uint32_t blx(uint32_t dstAddr, uint32_t srcAddr) {
    uint32_t offset = dstAddr - srcAddr;

    offset = (offset - 4) & 0x007fffff;


    uint32_t high = offset >> 12;

    uint32_t low = (offset & 0x00000fff) >> 1;


    if (low % 2 != 0) {

        low++;

    }


    uint32_t machineCode = ((0xEF00 | low) << 16) | (0xF000 | high);
    return machineCode;
}

uint32_t blx2(uint32_t dstAddr, uint32_t srcAddr) {
    uint32_t offset = dstAddr - srcAddr;

    offset = (offset - 4) & 0x007fffff;

    uint32_t high = offset >> 12;
    uint32_t low = (offset & 0x00000fff) >> 1;

    uint32_t machineCode = ((0xFF00 | low) << 16) | (0xF000 | high);
    return machineCode;
}

size_t *jnitrampolineAddress;

void (*artInterpreterToCompiledCodeBridge);

void hooktest() {
    JavaVM *vm = NULL;
    Environment::current()->GetJavaVM(&vm);
    JavaVMExt *ext = reinterpret_cast<JavaVMExt *>(vm);
    //读取runtime
    //vm 前四个字节
    char *runtimeAddress = (char *) ext->getRuntime();
    size_t pointerSize = sizeof(void *);
    RuntimeOffset runtimeOffset;

    {

        //runtime first
        loge("dodola", "runtimeAddress:%p", ext->getRuntime());
        size_t startOffset = (pointerSize == 4) ? 200 : 384;
        size_t endOffset = startOffset + (100 * pointerSize);
        int STD_STRING_SIZE = (pointerSize == 4) ? 12 : 24;


        for (size_t offset = startOffset; offset != endOffset; offset += pointerSize) {
            size_t *value = (size_t *) (runtimeAddress + offset);
            if (*value == (size_t) vm) {
                size_t classLinkerOffset = offset - STD_STRING_SIZE - (2 * pointerSize);
                size_t internTableOffset = classLinkerOffset - pointerSize;
                size_t threadListOffset = internTableOffset - pointerSize;

                size_t heapOffset = threadListOffset - (4 * pointerSize);
                heapOffset -= 3 * pointerSize;
//            if (apiLevel >= 24) {
//                heapOffset -= pointerSize;
//            }
                runtimeOffset.classLinker = classLinkerOffset;
                runtimeOffset.heapOffset = heapOffset;
                runtimeOffset.internTable = internTableOffset;
                runtimeOffset.threadList = threadListOffset;
                loge("dododola", "classLinker:%zu  heapOffset:%zu internTable:%zu threadList:%zu", classLinkerOffset,
                     heapOffset, internTableOffset, threadListOffset);


                break;
            }
        }
    }
    //获取方法统一执行入口
    ClassLinkerOffset classLinkerOffset;
    {
        size_t startOffset = (pointerSize == 4) ? 100 : 200;
        size_t endOffset = startOffset + (100 * pointerSize);
        size_t classLinker = (size_t) (runtimeAddress + runtimeOffset.classLinker);
        size_t internTable = (size_t) (runtimeAddress + runtimeOffset.internTable);

        size_t *internTableAddress = (size_t *) internTable;
        size_t *classLinkerAddress = (size_t *) classLinker;
        loge("dodola", "runtimeAddress:%p classLinker:%zu", ext->getRuntime(), *classLinkerAddress);


        for (size_t offset = startOffset; offset != endOffset; offset += pointerSize) {
            size_t *value = (size_t *) (*classLinkerAddress + offset);

            if (*value == *internTableAddress) {
                classLinkerOffset.quick_resolution_trampoline_ = offset + (pointerSize);
                classLinkerOffset.quick_generic_jni_trampoline_ = offset + (3 * pointerSize);
                classLinkerOffset.quick_to_interpreter_bridge_trampoline_ = offset + (4 * pointerSize);
                break;
            }
        }

    }
    size_t classLinker = (size_t) (runtimeAddress + runtimeOffset.classLinker);
    size_t *classLinkerAddress = (size_t *) classLinker;

    size_t *jnitrampoline = (size_t *) (*classLinkerAddress +
                                        classLinkerOffset.quick_generic_jni_trampoline_);
    jnitrampolineAddress = jnitrampoline;
    JNIEnv *env = Environment::current();
    void *handle = dlopen("libart.so", RTLD_LAZY | RTLD_GLOBAL);
    artInterpreterToCompiledCodeBridge = dlsym(handle,
                                               "artInterpreterToCompiledCodeBridge");
//    ZzHookPrePost((zpointer) *jnitrampoline, hook_pre_call, nullptr);
    loge("dodola", "runtimeAddress:%zu classLinker:%zu", *classLinkerAddress, *jnitrampoline);
}

jobject executeOrigin(JNIEnv *env, jobject objOrClass, size_t address) {

    //此处调用原来的
    return 0;
}

extern "C" void preCall() {
    loge("preCall", "cccccccccccccc");
}

extern "C" void postCall() {

}


extern "C" jobject JNICALL hookMethod(JNIEnv *env, jobject objOrClass, ...) {

//    int addressOrigin = 0x5678;

    preCall();
//    jobject result = executeOrigin(env, objOrClass, addressOrigin);
//    postCall();

    return env->NewStringUTF("olalalalal");
}


ArtMethodSpec getArtMethodSpec() {
    JNIEnv *env = Environment::current();

    jclass process = env->FindClass("android/os/Process");
    jmethodID setArgV0 = env->GetStaticMethodID(process, "setArgV0", "(Ljava/lang/String;)V");
    GumRuntimeBounds runtime_bounds;
    uint offset;
    size_t jniCodeOffset = NULL;
    size_t accessFlagsOffset = NULL;
    uint32_t expectedAccessFlags = kAccPublic | kAccStatic | kAccFinal | kAccNative;
    size_t entrypointFieldSize = 4;

    runtime_bounds.start = NULL;
    runtime_bounds.end = NULL;


    GumMemoryRange range;

    FILE *maps;

    int k, fd = -1, found = 0;
    char buff[256];
    GumAddress end;
    char path[256];
    char perms[5] = {0,};

    maps = fopen("/proc/self/maps", "r");
    char *libpath = "/system/lib/libandroid_runtime.so";

    while (!found && fgets(buff, sizeof(buff), maps)) {

        if (strstr(buff, "r-xp") && strstr(buff, libpath)) {
            loge("TAG", "============found %s", buff);
            int n = sscanf(buff,
                           "%"
                                   G_GINT64_MODIFIER
                                   "x-%"
                                   G_GINT64_MODIFIER
                                   "x "
                                   "%4c "
                                   "%*x %*s %*d "
                                   "%s",
                           &range.base_address, &end,
                           perms,
                           path);
            range.size = end - range.base_address;
            runtime_bounds.start = GSIZE_TO_POINTER (range.base_address);
            runtime_bounds.end = GSIZE_TO_POINTER (range.base_address + range.size);
            found = 1;
            break;
        }
    }

    fclose(maps);
    loge("TAG", "============found %d", found);


    int remaining = 2;
    for (offset = 0; offset != 64 && remaining != 0; offset += 4) {
        gpointer address = *((gpointer *) (GPOINTER_TO_SIZE(setArgV0) + offset));

        if (address >= runtime_bounds.start && address < runtime_bounds.end) {
            jniCodeOffset = offset;
            remaining--;
        }

        if (accessFlagsOffset == NULL) {
            uint32_t *flags = (uint32_t *) &address;//FIXME;
            if (*flags == expectedAccessFlags) {
                accessFlagsOffset = offset;
                remaining--;
            }
        }
    }
    size_t quickCodeOffset = (size_t) (jniCodeOffset + entrypointFieldSize);
    size_t size = quickCodeOffset + 4;
    ArtMethodSpec spec;
    spec.accessFlags = accessFlagsOffset;
    spec.jniCode = jniCodeOffset;
    spec.quickCode = quickCodeOffset;
    spec.size = size;
    spec.interpreterCode = jniCodeOffset - entrypointFieldSize;
    return spec;
}


void jni_testMethod(alias_ref<jclass>, jlong methodAddress, jint flags) {
    loge("dodola", "***********************");
    //判断是thumb还是art
    uint32_t hookMethodAddress = ((uint32_t) hookMethod);
    int runtimeType = hookMethodAddress & 1;
    loge("dodola", "=======  %s", runtimeType == 1 ? "thumb" : "art");
    ArtMethodSpec spec = getArtMethodSpec();
//    *((size_t *) (methodAddress + spec.jniCode)) = (size_t) getJumpPoint(methodAddress, (size_t) preCall,
//                                                                         (size_t) postCall,
//                                                                         (size_t) executeOrigin) + runtimeType ==
//                                                   1 ? 1 : 0;
//    ZzHookPrePost((void *) hookMethod, hook_pre_call, hook_post_call);

    *((size_t *) (methodAddress + spec.jniCode)) = (size_t) hookMethod;
    *((int *) (methodAddress + spec.accessFlags)) = kAccNative | kAccFastNative | flags;
    *((size_t *) (methodAddress + spec.quickCode)) = *jnitrampolineAddress;
    *((size_t *) (methodAddress + spec.interpreterCode)) = (size_t) artInterpreterToCompiledCodeBridge;

}

void jni_testMethod2(alias_ref<jclass>, jlong entryPointFromQuickCompiledCode) {
    ZzHookPrePost((void *) entryPointFromQuickCompiledCode, hook_pre_call, hook_post_call);
//    void *p = (void *) entryPointFromQuickCompiledCode;
//    ZzHookReplace((void *) entryPointFromQuickCompiledCode, (void *) hookMethod, nullptr);
}


jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    return initialize(vm, [] {

        loge("dodola", "===============hook JNI_OnLoad===========");
        nativeEngineClass = findClassStatic("profiler/dodola/lib/Profiler");
        nativeEngineClass->registerNatives({
                                                   makeNativeMethod("nativeEnableIOProfiler",
                                                                    jni_nativeEnableIOProfiler),
                                                   makeNativeMethod("startIOProfiler",
                                                                    jni_startIOProfiler),
                                                   makeNativeMethod("stopIOProfiler",
                                                                    jni_stopIOProfiler),
                                                   makeNativeMethod("testMethod", jni_testMethod),
                                                   makeNativeMethod("testMethod2", jni_testMethod2),

                                           });

        hooktest();

    });
}