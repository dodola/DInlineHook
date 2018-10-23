cmake_minimum_required(VERSION 3.6)

set(VIXL_SOURCE
        code-buffer-vixl.cc
        compiler-intrinsics-vixl.cc
        cpu-features.cc
        utils-vixl.cc
        aarch32/instructions-aarch32.cc
        aarch32/assembler-aarch32.cc
        aarch32/constants-aarch32.cc
        aarch32/disasm-aarch32.cc
        aarch32/location-aarch32.cc
        aarch32/macro-assembler-aarch32.cc
        aarch32/operands-aarch32.cc
        )

#set(WARNING_FLAGS "-Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -DVIXL_GENERATE_SIMULATOR_INSTRUCTIONS_VALUE=0  -fdiagnostics-show-option  -pedantic -DVIXL_CODE_BUFFER_MALLOC -DVIXL_INCLUDE_TARGET_T32")

add_library(vixl
        STATIC
        ${VIXL_SOURCE})

target_link_libraries(vixl)