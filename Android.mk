LOCAL_PATH := $(call my-dir)


####################################################################################################
#include $(CLEAR_VARS) 

LOCAL_MODULE := bootstrap

LOCAL_MODULE_FILENAME := libbootstrap

cmd-strip = $(TOOLCHAIN_PREFIX)strip --strip-unneeded -x $1

LOCAL_SRC_FILES := \
					xhook/xhook.c \
                    xhook/xh_core.c \
                    xhook/xh_elf.c \
                    xhook/xh_jni.c \
                    xhook/xh_log.c \
                    xhook/xh_util.c \
                    xhook/xh_version.c \
	bootstrap.cpp \
	zip/shadow_zip.cpp \
	zip/ZipEntry.cpp \
	zip/ZipFile.cpp

LOCAL_CFLAGS := -Izip -DTARGET_ARCH_ABI=\"$(TARGET_ARCH_ABI)\"  -DBUNDLE_ID=\"com.test.test\"  -std=c11 -fvisibility=hidden
LOCAL_CPPFLAGS := -Izip -DTARGET_ARCH_ABI=\"$(TARGET_ARCH_ABI)\"  -DBUNDLE_ID=\"com.test.test\"  -std=c++11 -fvisibility=hidden

LOCAL_LDLIBS := -llog -L$(LOCAL_PATH)/../../libs/$(TARGET_ARCH_ABI)

LOCAL_C_INCLUDES  += system/core/include/cutils

LOCAL_SHARED_LIBRARIES := 

include $(BUILD_SHARED_LIBRARY)

####################################################################################################