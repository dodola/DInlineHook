cmake_minimum_required(VERSION 3.6)

set(HOOK_SOURCES
    IOHooker.cpp
)

link_libraries(hookzz)
link_libraries(utils)

add_library(profiler_hook
           STATIC
            ${HOOK_SOURCES})

target_link_libraries(profiler_hook )