cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )

#set(WARNING_FLAGS "-Wall -Werror")
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} -std=c11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -std=c++11 -O3")


message('currrent file location:${CMAKE_CURRENT_SOURCE_DIR}')
#add_library(luajit STATIC IMPORTED)
#set_target_properties(luajit
#        PROPERTIES IMPORTED_LOCATION
#        ${CMAKE_CURRENT_SOURCE_DIR}/luajava/${ANDROID_ABI}/libluajit.a)

find_library(atomic-lib
        atomic)

include_directories(fb/include)
include_directories(utils)
include_directories(vixl)

add_subdirectory(utils)
add_subdirectory(fb)
add_subdirectory(jniwrapper)
add_subdirectory(vixl)


add_library(dodo
        SHARED
        Ding.cpp
        )
target_link_libraries(dodo vixl fbjni jni_wrapper)