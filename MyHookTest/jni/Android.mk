LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
                test.cpp \

LOCAL_MODULE := test

LOCAL_LDLIBS += -llog

LOCAL_CPPFLAGS	:= -std=gnu++11 -fpermissive

#include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
