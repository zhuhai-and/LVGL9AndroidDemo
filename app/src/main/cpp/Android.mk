# Android.mk - ndk-build 构建脚本
# 编译 LVGL 源码、官方 Demo、象棋 Demo 和 JNI 桥接代码为 liblvApp.so

LOCAL_PATH := $(call my-dir)
LV_PATH := $(LOCAL_PATH)/lvgl-9.5.0/

LOCAL_SHORT_COMMANDS := true
LOCAL_ARM_NEON := true
# build shared libs
include $(CLEAR_VARS)
LOCAL_MODULE := lvApp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/lvapp \
    $(LV_PATH) \
    $(LV_PATH)/demos

LOCAL_SRC_FILES := \
	$(wildcard $(LV_PATH)/src/*.c) \
	$(wildcard $(LV_PATH)/src/**/*.c) \
	$(wildcard $(LV_PATH)/src/**/**/*.c) \
	$(wildcard $(LV_PATH)/src/extra/**/**/*.c) \
	$(wildcard $(LV_PATH)/src/misc/**/**/*.c) \
	$(wildcard $(LV_PATH)/src/draw/**/**/*.c) \
	$(wildcard $(LV_PATH)/demos/*.c) \
	$(wildcard $(LV_PATH)/demos/**/*.c) \
	$(wildcard $(LV_PATH)/demos/**/**/*.c) \
	$(wildcard $(LOCAL_PATH)/**/*.c) \
	$(wildcard $(LOCAL_PATH)/chess/imgs/*.c) \
	$(wildcard $(LOCAL_PATH)/chess/*.cpp) \
	$(wildcard $(LOCAL_PATH)/*.cpp)

# 编译选项：函数/数据段分离 + 隐藏符号可见性，便于链接器裁剪未使用代码
LOCAL_CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden
# 链接选项：16KB 页对齐（Android 15 要求）+ 裁剪未使用段
LOCAL_LDFLAGS += -Wl,-z,max-page-size=16384
LOCAL_LDFLAGS += -Wl,--gc-sections
LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)