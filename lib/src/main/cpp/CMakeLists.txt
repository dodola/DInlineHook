cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    )

#set(WARNING_FLAGS "-Wall -Werror")
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -std=c11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -std=c++11 -O3")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")

find_library(atomic-lib
            atomic)

include_directories(fb/include)
include_directories(HookZz/include)
include_directories(HookZz/src)
include_directories(HookZz/src/kitzz/include)
include_directories(HookZz/src/kitzz/CommonKit)
include_directories(HookZz/src/kitzz)

include_directories(utils)


include_directories(iohook)

add_subdirectory(utils)
add_subdirectory(fb)
add_subdirectory(HookZz)
add_subdirectory(iohook)
add_subdirectory(jniwrapper)
add_subdirectory(epic)


#add_subdirectory(perfd)


add_library(dodo
SHARED
Ding.cpp
)
target_link_libraries(dodo fbjni hookzz profiler_hook jni_wrapper epic)