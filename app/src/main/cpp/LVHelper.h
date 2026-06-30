#ifndef LVGLPORTANDROID_LVHELPER_H
#define LVGLPORTANDROID_LVHELPER_H

#include <jni.h>

/**
 * Native 日志宏定义。
 *
 * 在 Release 构建（定义了 NDEBUG）时，所有日志宏被禁用为空操作，
 * 避免日志输出带来的性能开销。
 * 在 Debug 构建时，日志通过 Android logcat 输出，TAG 为 "NATIVE.LOG"。
 */
#ifdef NDEBUG
#define LOGD(...) do{}while(0)
#define LOGI(...) do{}while(0)
#define LOGW(...) do{}while(0)
#define LOGE(...) do{}while(0)
#define LOGF(...) do{}while(0)
#else
#define LOG_TAG "NATIVE.LOG"

#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,LOG_TAG,__VA_ARGS__)
#endif

#endif //LVGLPORTANDROID_LVHELPER_H
