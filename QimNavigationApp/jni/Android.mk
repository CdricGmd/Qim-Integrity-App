LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include /Users/Cedric/Documents/Dev/OpenCV/OpenCV-2.4.6-android-sdk/sdk/native/jni/OpenCV.mk
LOCAL_MODULE    := QimNativeEngine
LOCAL_SRC_FILES := QimNativeEngine.cpp mQim.cpp measurements.cpp lsb.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := -lisbstl #-libmeasurements -libmqim -liblsb
LOCAL_CFLAGS    := -Werror
LOCAL_LDLIBS +=  -llog -ldl
include $(BUILD_SHARED_LIBRARY)
LOCAL_PATH:= $(call my-dir)