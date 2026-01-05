
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := higgshook
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS    := -llog -landroid

# Include Dobby
LOCAL_C_INCLUDES := $(LOCAL_PATH)/Dobby/include
LOCAL_LDFLAGS    := -L$(LOCAL_PATH)/Dobby/lib/$(TARGET_ARCH_ABI) -ldobby

include $(BUILD_SHARED_LIBRARY)
