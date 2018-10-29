//
// Created by didi on 2017/10/30.
//

#include "Ding.h"
#include <fb/Build.h>
#include <fb/ALog.h>
#include <fb/fbjni.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>

extern "C" {
#include <stdint.h>
}

#include <cstdio>
#include <string>
#include <iostream>

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


int kAccPublic = 0x0001;
int kAccStatic = 0x0008;
int kAccFinal = 0x0010;
int kAccNative = 0x0100;
int kAccFastNative = 0x00080000;


alias_ref<jclass> nativeEngineClass;


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



size_t *jnitrampolineAddress;

void (*artInterpreterToCompiledCodeBridge);

void initHook() {
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
                loge("dododola", "classLinker:%zu  heapOffset:%zu internTable:%zu threadList:%zu",
                     classLinkerOffset,
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
                classLinkerOffset.quick_to_interpreter_bridge_trampoline_ =
                        offset + (4 * pointerSize);
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
    loge("dodola", "runtimeAddress:%zu classLinker:%zu", *classLinkerAddress, *jnitrampoline);
}


extern "C" jobject JNICALL
hookMethod(JNIEnv *env, jobject objOrClass, RegisterContext *reg, HookInfo *info) {
//    //调用原方法
//    jmethodID art_method = env->FromReflectedMethod(info->reflectedMethod);
    loge("dodola", "========call hook method========= ");
//
//    jstring strresult = static_cast<jstring>(env->CallObjectMethod(objOrClass,
//                                                             reinterpret_cast<jmethodID>(art_method)));
//    jstring originStr = static_cast<jstring>(env->CallObjectMethod(objOrClass, art_method));
//
//    loge("dodola", "========call hook method=========%d %s", art_method, originStr);


    jclass innerHooker = env->FindClass("profiler/dodola/lib/InnerHooker");
    jmethodID callOrignMethod = env->GetStaticMethodID(innerHooker, "callOrigin",
                                                       "(Ljava/lang/reflect/Member;Ljava/lang/Object;)V");
    env->CallStaticVoidMethod(innerHooker, callOrignMethod, nullptr, (jobject*)reg->general.regs.r2);

//    const char *originChars = env->GetStringUTFChars(originStr, 0);
    jstring str = env->NewStringUTF("==========");
//    loge("dodola", "getresultsssssss %d %d %s", reg->general.regs.r0, env, originChars);
    return str;
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
    size_t entrypointFieldSize = 4;//apiLevel<=21 ?8 :4

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
    loge("TAG", "============make spec =========");
    size_t quickCodeOffset = (size_t) (jniCodeOffset + entrypointFieldSize);
    size_t size = quickCodeOffset + 4;
    ArtMethodSpec spec;
    spec.accessFlags = accessFlagsOffset;
    spec.jniCode = jniCodeOffset;
    spec.quickCode = quickCodeOffset;
    spec.size = size;
    spec.interpreterCode = jniCodeOffset - entrypointFieldSize;
    loge("TAG", "============make spec end =========");

    return spec;
}


static HookInfo *enableHook(void *art_method, jobject additional_info) {
    JNIEnv *env = Environment::current();

    jclass java_lang_reflect_Method = env->FindClass("java/lang/reflect/Method");
    jclass java_lang_reflect_AbstractMethod = env->FindClass("java/lang/reflect/Executable");
    jfieldID java_lang_reflect_AbstractMethod_artMethod = env->GetFieldID(
            java_lang_reflect_AbstractMethod, "artMethod", "J");
    //备份原来的方法
    ArtMethodSpec spec = getArtMethodSpec();
    void *backupMethod = malloc(spec.size);
    memcpy(backupMethod, art_method, spec.size);
    loge("dodola", "backupMethod %d", backupMethod);

    jobject reflect_method = env->AllocObject(java_lang_reflect_Method);
    env->SetLongField(reflect_method, java_lang_reflect_AbstractMethod_artMethod,
                      (jlong) backupMethod);
    HookInfo *hookInfo = reinterpret_cast<HookInfo *>(calloc(1, sizeof(HookInfo)));
    hookInfo->reflectedMethod = env->NewGlobalRef(reflect_method);
    hookInfo->additionalInfo = env->NewGlobalRef(additional_info);
    return hookInfo;

}


#define __ masm->

void GenerateDemo(MacroAssembler *masm) {
    __ Ldr(r1, 1);
    __ Add(r0, r0, r1);
    __ Bx(lr);
}


void generatorJumpMethod(HookInfo *hookInfo, MacroAssembler *masm) {
    loge("dodola", "hookMethod %x", (uint32_t) hookMethod);


    __ Sub(sp, sp, Operand(14 * 4));
    __ Str(lr, MemOperand(sp, 13 * 4));
    __ Str(r12, MemOperand(sp, 12 * 4));
    __ Str(r11, MemOperand(sp, 11 * 4));
    __ Str(r10, MemOperand(sp, 10 * 4));
    __ Str(r9, MemOperand(sp, 9 * 4));
    __ Str(r8, MemOperand(sp, 8 * 4));
    __ Str(r7, MemOperand(sp, 7 * 4));
    __ Str(r6, MemOperand(sp, 6 * 4));
    __ Str(r5, MemOperand(sp, 5 * 4));
    __ Str(r4, MemOperand(sp, 4 * 4));
    __ Str(r3, MemOperand(sp, 3 * 4));
    __ Str(r2, MemOperand(sp, 2 * 4));
    __ Str(r1, MemOperand(sp, 1 * 4));
    __ Str(r0, MemOperand(sp, 0 * 4));
    __ Sub(sp, sp, Operand(8));
    //要保留r0 和 r1 寄存器，r0=>env r1 => class or obj r2=>RegisterContext r3 hook info
    __ Mov(r2, sp);
    __ Mov(r3, (uint32_t) hookInfo);
    __ Mov(r4, (uint32_t) hookMethod);
    __ Blx(r4);
    //需要还原lr
    __ Add(sp, sp, Operand(8));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(r1, MemOperand(sp, 4, PostIndex));
    __ Ldr(lr, MemOperand(sp, 4, PostIndex));
    __ Bx(lr);

}


uint32_t gens(HookInfo *hookInfo) {

    MacroAssembler masm(A32);
    Label demo;
    masm.Bind(&demo);
    generatorJumpMethod(hookInfo, &masm);
    masm.FinalizeCode();

    byte *code = masm.GetBuffer()->GetStartAddress<byte *>();
    uint32_t code_size = masm.GetSizeOfCodeGenerated();
    ExecutableMemory memory(code, code_size);
    uint32_t
    (*demo_function)(JNIEnv, jobject, ...) = memory.GetEntryPoint<uint32_t (*)(JNIEnv, jobject,
                                                                               ...)>
            (demo, masm.GetInstructionSetInUse());
    return reinterpret_cast<uint32_t>(demo_function);

}


void testVixl() {
    loge("dodola", "===============testVixl===========");

    MacroAssembler masm;
    Label demo;
    masm.Bind(&demo);
    GenerateDemo(&masm);
    masm.FinalizeCode();
    std::cout << std::endl;
    std::cout << "Standard disassembly:" << std::endl;
    PrintDisassembler print_disassembler(std::cout);
    print_disassembler.DisassembleA32Buffer(masm.GetBuffer()->GetOffsetAddress<uint32_t *>(0),
                                            masm.GetBuffer()->GetSizeInBytes());

    byte *code = masm.GetBuffer()->GetStartAddress<byte *>();
    uint32_t code_size = masm.GetSizeOfCodeGenerated();
    ExecutableMemory memory(code, code_size);
    uint32_t (*demo_function)(uint32_t) = memory.GetEntryPoint<uint32_t (*)(
            uint32_t)>(demo, masm.GetInstructionSetInUse());
    uint32_t input_value = 2;
    uint32_t output_value = (*demo_function)(input_value);
    loge("dodola", "native: demo(0x%08x) = 0x%08x\n", input_value, output_value);
}


void jni_testMethod(alias_ref<jclass>, jobject method, jint flags) {
    JNIEnv *env = Environment::current();
    jlong methodAddress = (jlong) env->FromReflectedMethod(method);

    //判断是thumb还是art
//    uint32_t hookMethodAddress = ((uint32_t) hookMethod);
    uint32_t hookMethodAddress = gens(enableHook(reinterpret_cast<void *>(methodAddress), nullptr));
    int runtimeType = hookMethodAddress & 1;
    loge("dodola", "=======  %s", runtimeType == 1 ? "thumb" : "art");
    ArtMethodSpec spec = getArtMethodSpec();
    *((size_t *) (methodAddress + spec.jniCode)) = (size_t) hookMethodAddress;
    loge("dodola", "***********************begin*********************");

    *((int *) (methodAddress + spec.accessFlags)) = kAccNative | kAccFastNative | flags;

    *((size_t *) (methodAddress + spec.quickCode)) = *jnitrampolineAddress;
    *((size_t *) (methodAddress +
                  spec.interpreterCode)) = (size_t) artInterpreterToCompiledCodeBridge;

    loge("dodola", "**********************end**********************");

}


jlong jni_getMethodAddress(alias_ref<jclass>, jobject method) {
    JNIEnv *env = Environment::current();

    jlong art_method = (jlong) env->FromReflectedMethod(method);
    return art_method;
}

void jni_memput(alias_ref<jclass>, jbyteArray src, jlong dest) {
    JNIEnv *env = Environment::current();

    jbyte *srcPnt = env->GetByteArrayElements(src, 0);
    jsize length = env->GetArrayLength(src);
    unsigned char *destPnt = (unsigned char *) dest;
    for (int i = 0; i < length; ++i) {
        destPnt[i] = (unsigned char) srcPnt[i];
    }
    env->ReleaseByteArrayElements(src, srcPnt, 0);
}

jbyteArray jni_memget(alias_ref<jclass>, jlong src, jint length) {
    JNIEnv *env = Environment::current();

    jbyteArray dest = env->NewByteArray(length);
    if (dest == NULL) {
        return NULL;
    }
    unsigned char *destPnt = (unsigned char *) env->GetByteArrayElements(dest, 0);
    unsigned char *srcPnt = (unsigned char *) src;
    for (int i = 0; i < length; ++i) {
        destPnt[i] = srcPnt[i];
    }
    env->ReleaseByteArrayElements(dest, (jbyte *) destPnt, 0);

    return dest;
}


jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    return initialize(vm, [] {

        std::cout.rdbuf(new androidbuf);

        loge("dodola", "===============hook JNI_OnLoad===========");
        nativeEngineClass = findClassStatic(
                "profiler/dodola/lib/InnerHooker");
        nativeEngineClass->registerNatives({
                                                   makeNativeMethod("testMethod", jni_testMethod),
                                                   makeNativeMethod("memput", jni_memput),
                                                   makeNativeMethod("memget", jni_memget),
                                                   makeNativeMethod("getMethodAddress",
                                                                    jni_getMethodAddress)
                                           });
        initHook();
    });
}


