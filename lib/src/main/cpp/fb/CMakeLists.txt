cmake_minimum_required(VERSION 3.6)

set(FB_SOURCE
       fbassert.cpp
       jni/ByteBuffer.cpp
       jni/Countable.cpp
       jni/Environment.cpp
       jni/Exceptions.cpp
       jni/fbjni.cpp
       jni/Hybrid.cpp
       jni/jni_helpers.cpp
       jni/LocalString.cpp
       jni/OnLoad.cpp
       jni/References.cpp
       jni/WeakReference.cpp
       log.cpp
       lyra/lyra.cpp
       onload.cpp
)

#set(WARNING_FLAGS "-Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_TAG=\"libfb\" -DDISABLE_CPUCAP -DDISABLE_XPLAT -fexceptions  -DHAVE_POSIX_CLOCKS -frtti -Wno-unused-parameter ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -DDISABLE_CPUCAP -DDISABLE_XPLAT -fexceptions -DHAVE_POSIX_CLOCKS   -std=gnu++11")


find_library( log-lib
              log )
find_library( dl-lib
              dl )
find_library( android-lib
              android )

add_library(fbjni
           STATIC
            ${FB_SOURCE})

target_link_libraries(fbjni ${log-lib} ${dl-lib} ${android-lib} )