cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 14)
enable_language(ASM)
SET(CMAKE_ASM_FLAGS "${CFLAGS}  -x assembler-with-cpp")

set(FFI_SOURCE
        closures.c
        debug.c
        dlmalloc.c
        java_raw_api.c
        prep_cif.c
        raw_api.c
        types.c
        ffi_cxx.cc
        )
include_directories(.)
include_directories(platform_include)
add_library(ffi
        STATIC
        ${FFI_SOURCE})

target_link_libraries(ffi)