cmake_minimum_required(VERSION 3.6)

set(UTILS_WRAPPER
    java_lang_wrapper.cpp
    java_lang_wrapper.h
)
#include_directories(.)
add_library(jni_wrapper
           STATIC
            ${UTILS_WRAPPER})

target_link_libraries(jni_wrapper )