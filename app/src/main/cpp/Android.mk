LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    :=  native-lib
LOCAL_SRC_FILES := native-lib.cpp
LOCAL_CFLAGS := -std=gnu++11

ifeq ($(TARGET_ARCH),arm)
LOCAL_SRC_FILES +=test.s
endif
LOCAL_LDLIBS    := -llog -landroid
include $(BUILD_SHARED_LIBRARY)