/**
 * JNI 入口文件，实现 LVApp.java 中声明的所有 native 方法。
 *
 * 每个 native 方法接收一个 jlong 类型的实例 ID（即 LVApp* 指针），
 * 通过强制转换获取 C++ 对象后调用对应方法。
 */
#include <iostream>
#include <lvgl.h>
#include <lv_demos.h>
#include <jni.h>
#include <thread>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "LVApp.h"
#include "LVHelper.h"


// JNI 函数名宏：展开为 Java_com_hzy_lvgl_demo_LVApp_<name>
#define J_FUN(r, x) JNIEXPORT r JNICALL Java_com_hzy_lvgl_demo_LVApp_##x

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

/**
 * JNI 库加载回调，在 System.loadLibrary 后由 JVM 调用。
 * 返回 JNI 版本号以声明使用的 JNI API 级别。
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    LOGD("JNI_OnLoad()");
    return JNI_VERSION_1_4;
}

/** 创建 LVApp 实例，返回指针供 Java 层持有 */
J_FUN(jlong, nCreate)(JNIEnv *env, jclass clazz) {
    return (jlong) new LVApp();
}

/** 设置要运行的 Demo 名称 */
J_FUN(jlong, nSetApp)(JNIEnv *env, jclass clazz, jlong id, jstring _name) {
    if (id == 0 || _name == nullptr) {
        return -1;
    }
    auto *name = env->GetStringUTFChars(_name, nullptr);
    if (name == nullptr) {
        return -1;
    }
    auto *p = (LVApp *) id;
    p->setApp(name);
    env->ReleaseStringUTFChars(_name, name);
    return 0;
}

/** 设置 LVGL 逻辑分辨率 */
J_FUN(jlong, nSetSize)(JNIEnv *env, jclass clazz, jlong id, jint w, jint h) {
    if (id == 0 || w <= 0 || h <= 0) {
        return -1;
    }
    auto *p = (LVApp *) id;
    p->setSize(w, h);
    return 0;
}

/** 启动 LVGL 渲染，将 Java Surface 转换为 ANativeWindow 后传给 LVApp */
J_FUN(jlong, nStart)(JNIEnv *env, jclass clazz, jlong id, jobject surface) {
    if (id == 0 || surface == nullptr) {
        return -1;
    }
    auto *p = (LVApp *) id;
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        return -1;
    }
    // ANativeWindow_fromSurface 返回的引用由 LVApp 接管，并在渲染线程退出时释放。
    p->start(window);
    return 0;
}

/** 转发触摸事件到 Native 层 */
J_FUN(jlong, nOnTouch)(JNIEnv *env, jclass clazz, jlong id, jint touch, jint x, jint y) {
    if (id == 0) {
        return -1;
    }
    auto *p = (LVApp *) id;
    p->onTouch(touch, x, y);
    return 0;
}

/** 停止渲染线程（暂停渲染，不销毁实例） */
J_FUN(jlong, nStop)(JNIEnv *env, jclass clazz, jlong id) {
    if (id == 0) {
        return -1;
    }
    auto *p = (LVApp *) id;
    p->stop();
    return 0;
}

/** 销毁 LVApp 实例，释放所有 Native 资源 */
J_FUN(jlong, nDestroy)(JNIEnv *env, jclass clazz, jlong id) {
    if (id == 0) {
        return -1;
    }
    auto *p = (LVApp *) id;
    delete p;
    return 0;
}

#ifdef __cplusplus
}
#endif
