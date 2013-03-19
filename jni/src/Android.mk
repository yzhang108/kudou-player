LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
FFMPEG_PATH = ../ffmpeg

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
					$(LOCAL_PATH)/$(FFMPEG_PATH)/include \
					$(LOCAL_PATH)/include

# Add your application source files here...
#LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp   play.c native.cpp
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp   play.c native.cpp

LOCAL_CFLAGS += -DANDROID
LOCAL_SHARED_LIBRARIES := SDL

LOCAL_LDLIBS := -lGLESv1_CM -llog
LOCAL_LDLIBS += $(LOCAL_PATH)/"libffmpeg.so"
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := ffmpeg
NDK_MODULE_PATH := $(LOCAL_PATH)
LOCAL_SRC_FILES := libffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)
