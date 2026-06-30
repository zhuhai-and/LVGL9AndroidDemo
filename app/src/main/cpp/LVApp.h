#ifndef LVGLPORTANDROID_LVAPP_H
#define LVGLPORTANDROID_LVAPP_H

#include <jni.h>
#include <atomic>
#include <string>
#include <thread>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <lvgl.h>
#include "AppList.h"

/**
 * LVGL 应用核心类，管理 LVGL 在 Android 上的完整生命周期。
 *
 * 每个 LVApp 实例拥有一个独立的渲染线程，在该线程内运行 LVGL 主循环。
 * Java 层通过 JNI 调用此类的方法来控制 LVGL 的启动、停止、Demo 切换和触摸输入。
 *
 * 线程安全说明：
 *   - isTouch/touchX/touchY/is_running 使用 atomic 保证跨线程读写安全
 *   - LVGL API 调用全部在渲染线程内完成，不需要额外锁
 *   - start/stop 方法通过 join 保证线程同步
 */
class LVApp {
private:
    ANativeWindow *window = nullptr;          // Android 原生窗口引用
    ANativeWindow_Buffer windowBuffer;         // NativeWindow 帧缓冲信息
    int app_width = 480, app_height = 320;     // LVGL 逻辑分辨率
    int screen_width = 0, screen_height = 0;   // 物理屏幕尺寸（用于触摸坐标映射）
    uint16_t *surface_buf = nullptr;           // 全帧累积缓冲区（RGB565），
                                               // ANativeWindow 使用多缓冲机制，
                                               // 每次锁定可能返回不同缓冲区，
                                               // 需要持久缓冲区累积局部刷新后整体拷贝
    size_t surface_size = 0;                    // 全帧累积缓冲区大小
    string app_name;                            // 当前运行的 Demo 名称
    atomic<int> isTouch = 0;                   // 触摸状态：1=按下，0=释放
    atomic<int> touchX = 0;                    // 触摸 X 坐标（已映射为 LVGL 逻辑坐标）
    atomic<int> touchY = 0;                    // 触摸 Y 坐标（已映射为 LVGL 逻辑坐标）
    atomic<bool> is_running = false;           // 渲染线程运行标志
    std::thread m_thread;                       // LVGL 渲染线程

    // LVGL 显示刷新回调（实例方法），将局部区域像素写入 NativeWindow
    void lv_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

    // LVGL 触摸输入读取回调（实例方法），向 LVGL 报告当前触摸状态
    void lv_touch_callback(lv_indev_t *indev_drv, lv_indev_data_t *data);

    // LVGL 主循环任务，运行在独立线程中
    void lv_loop_task();

    // LVGL tick 回调，返回当前系统时间戳（毫秒）
    static uint32_t lv_tick_get();

    // LVGL 日志回调，转发到 Android logcat
    static void lv_log_print(lv_log_level_t level, const char *buf);

    // 静态刷新回调包装，通过 user_data 转发到实例方法
    static void lv_flush_cb_static(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

    // 静态触摸回调包装，通过 user_data 转发到实例方法
    static void lv_touch_cb_static(lv_indev_t *indev_drv, lv_indev_data_t *data);

public:
    /** 析构函数，停止渲染线程并释放资源 */
    ~LVApp();

    /** 启动 LVGL 渲染，在独立线程中运行主循环 */
    void start(ANativeWindow *window);

    /** 设置要运行的 Demo 名称（需与 AppList.h 中的 key 一致） */
    void setApp(const char *name);

    /** 处理触摸事件，将屏幕坐标映射为 LVGL 逻辑坐标 */
    void onTouch(int touch, int x, int y);

    /** 设置 LVGL 逻辑分辨率，需在 start() 之前调用 */
    void setSize(int w, int h);

    /** 停止渲染线程，等待线程退出 */
    void stop();
};


#endif //LVGLPORTANDROID_LVAPP_H
