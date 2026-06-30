#include <thread>
#include <mutex>
#include "LVApp.h"
#include "LVHelper.h"
#include <lv_demos.h>
#include <unistd.h>

using namespace std;

/**
 * LVGL 显示刷新回调（实例方法）。
 *
 * LVGL 在 PARTIAL 渲染模式下，每次完成一块区域的绘制后会调用此回调。
 * 回调负责将 px_map 中的局部像素数据写入 ANativeWindow 的帧缓冲。
 *
 * 实现流程：
 *   1. 懒分配 surface_buf（与 NativeWindow 帧缓冲等大的全帧累积缓冲）
 *   2. 将局部区域逐行拷贝到 surface_buf 对应位置（累积完整帧）
 *   3. 锁定 NativeWindow，将 surface_buf 整体拷贝到帧缓冲，解锁并提交
 *   4. 通知 LVGL 刷新完成（lv_disp_flush_ready）
 *
 * 注意：ANativeWindow 使用多缓冲机制，每次 lock 可能返回不同缓冲区。
 * 必须用 surface_buf 累积完整帧后整体拷贝，否则部分缓冲区只收到局部更新
 * 会导致显示闪烁或残缺。
 *
 * @param disp  LVGL 显示对象
 * @param area  本次刷新的矩形区域（LVGL 逻辑坐标）
 * @param px_map LVGL 绘制缓冲，包含 area 区域内的像素数据（RGB565 格式）
 */
void LVApp::lv_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    if (is_running && window != nullptr) {
        // 懒分配全帧累积缓冲区，大小与 NativeWindow 帧缓冲一致
        if (surface_buf == nullptr) {
            surface_size = sizeof(uint16_t) * windowBuffer.stride * windowBuffer.height;
            surface_buf = (uint16_t *) malloc(surface_size);
        }
        int w = area->x2 - area->x1 + 1;
        int h = area->y2 - area->y1 + 1;
        auto *src = (uint16_t *) px_map;
        auto stride = windowBuffer.stride;
        // 将局部区域数据逐行拷贝到累积缓冲的对应位置
        for (int i = 0; i < h; i++) {
            auto *dst = &surface_buf[(area->y1 + i) * stride + area->x1];
            memcpy(dst, &src[i * w], sizeof(uint16_t) * w);
        }
        // 锁定帧缓冲，整体拷贝后提交显示
        if (ANativeWindow_lock(window, &windowBuffer, nullptr) == 0) {
            memcpy(windowBuffer.bits, surface_buf, surface_size);
            ANativeWindow_unlockAndPost(window);
        }
    }
    // 通知 LVGL 本次刷新已完成，可以继续下一帧
    lv_disp_flush_ready(disp);
}

/**
 * LVGL 触摸输入读取回调（实例方法）。
 *
 * LVGL 主循环每次调用 lv_timer_handler 时会定期读取输入设备状态。
 * 此回调将 Java 层转发来的触摸坐标和按下/释放状态传递给 LVGL。
 *
 * @param indev_drv LVGL 输入设备对象
 * @param data      输出参数，填充当前触摸状态和坐标
 */
void LVApp::lv_touch_callback(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    if (isTouch) {
        data->point.x = (short) touchX;
        data->point.y = (short) touchY;
        data->state = LV_INDEV_STATE_PR;  // 按下状态
    } else {
        data->state = LV_INDEV_STATE_REL;  // 释放状态
    }
}

/**
 * LVGL 日志回调，将 LVGL 内部日志转发到 Android logcat。
 *
 * @param level 日志级别
 * @param buf   日志内容
 */
void LVApp::lv_log_print(lv_log_level_t level, const char *buf) {
    switch (level) {
        case LV_LOG_LEVEL_INFO:
            LOGI("%s", buf);
            break;
        case LV_LOG_LEVEL_WARN:
            LOGW("%s", buf);
            break;
        case LV_LOG_LEVEL_ERROR:
            LOGE("%s", buf);
            break;
        case LV_LOG_LEVEL_TRACE:
            LOGD("%s", buf);
            break;
        default:
            LOGI("%s", buf);
    }
}

/**
 * LVGL tick 获取回调，返回当前系统时间戳（毫秒）。
 * 使用 gettimeofday 获取高精度时间，作为 LVGL 的心跳源。
 *
 * @return 当前时间戳（毫秒）
 */
uint32_t LVApp::lv_tick_get() {
    static struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/**
 * 启动 LVGL 渲染。
 *
 * 如果已有渲染线程在运行，先停止并等待退出。然后设置 NativeWindow 的几何属性
 * （分辨率和像素格式为 RGB_565），最后在独立线程中启动 LVGL 主循环。
 *
 * @param win 由 ANativeWindow_fromSurface 获取的 NativeWindow 引用，
 *            调用者负责在渲染线程退出时 release
 */
void LVApp::start(ANativeWindow *win) {
    stop();  // 确保之前的渲染线程已退出
    if (win == nullptr) {
        return;
    }
    this->window = win;
    // 记录物理屏幕尺寸，用于触摸坐标映射
    screen_width = ANativeWindow_getWidth(window);
    screen_height = ANativeWindow_getHeight(window);
    // 设置 NativeWindow 缓冲区几何属性：使用 LVGL 逻辑分辨率 + RGB_565 格式
    if (ANativeWindow_setBuffersGeometry(window, app_width, app_height, WINDOW_FORMAT_RGB_565) != 0) {
        LOGE("Failed to set NativeWindow geometry [%d x %d]", app_width, app_height);
        ANativeWindow_release(window);
        window = nullptr;
        return;
    }
    LOGD("LV Screen [%d x %d]", app_width, app_height);
    is_running = true;
    m_thread = thread(&LVApp::lv_loop_task, this);
}

/**
 * LVGL 主循环任务，运行在独立的渲染线程中。
 *
 * 执行流程：
 *   1. 初始化 LVGL 核心（lv_init）、设置 tick 和日志回调
 *   2. 创建显示对象，绑定刷新回调和绘制缓冲
 *   3. 创建触摸输入设备，绑定读取回调
 *   4. 根据 app_name 从 AppList 映射表中选择并启动对应 Demo
 *   5. 进入主循环，定期调用 lv_timer_handler 驱动 LVGL 刷新
 *   6. 收到停止信号后清理资源并退出
 *
 * 注意：LVGL 非线程安全，所有 LVGL API 调用都在此线程内完成。
 */
void LVApp::lv_loop_task() {
    LOGD("LV Task Start!!");
    lv_init();
    lv_tick_set_cb(lv_tick_get);
    lv_log_register_print_cb(lv_log_print);

    // 创建显示对象，设置用户数据为 this 以便在静态回调中获取实例
    auto *disp = lv_display_create(app_width, app_height);
    lv_display_set_user_data(disp, this);
    lv_display_set_flush_cb(disp, LVApp::lv_flush_cb_static);

    // 分配绘制缓冲区（PARTIAL 模式），大小为 1 倍屏幕像素数（RGB565 = 2 字节/像素）
    // 之前使用 10 倍缓冲约 1.5MB，优化后仅需约 300KB，显著降低内存占用
    size_t buf_size = (size_t) app_width * app_height * 2;
    auto *buf = malloc(buf_size);
    lv_display_set_buffers(disp, buf, nullptr, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 创建触摸输入设备（指针类型）
    auto *indev = lv_indev_create();
    lv_indev_set_user_data(indev, this);
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, LVApp::lv_touch_cb_static);

    // 从映射表中查找要启动的 Demo，未找到则使用默认（widgets）
    auto *lv_app_func = lv_ci_example_list.at("default");
    auto search = lv_ci_example_list.find(app_name);
    if (search != lv_ci_example_list.end()) {
        lv_app_func = search->second;
    }
    lv_app_func();

    // 首帧前先提交一次空帧，确保 NativeWindow 缓冲区已初始化
    if (ANativeWindow_lock(window, &windowBuffer, nullptr) == 0) {
        ANativeWindow_unlockAndPost(window);
    }

    // LVGL 主循环：lv_timer_handler 返回下次需要运行的时间（毫秒），
    // 据此动态调整休眠时长，避免不必要的 CPU 唤醒
    while (is_running) {
        uint32_t time_till_next = lv_timer_handler();
        // 限制休眠范围：最少 1ms 避免忙等，最多 33ms（约 30fps）
        if (time_till_next < 1) time_till_next = 1;
        if (time_till_next > 33) time_till_next = 33;
        usleep(time_till_next * 1000);
    }

    // 清理资源
    ANativeWindow_release(window);
    lv_deinit();
    free(buf);
    if (surface_buf != nullptr) {
        free(surface_buf);
        surface_buf = nullptr;
    }
    window = nullptr;
    LOGD("LV App Stopped!!");
}

/**
 * 停止渲染线程。设置运行标志为 false 并等待线程退出。
 * 线程退出后会自动 release NativeWindow 引用。
 */
void LVApp::stop() {
    is_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/**
 * 处理来自 Java 层的触摸事件。
 * 将屏幕物理坐标按比例映射为 LVGL 逻辑坐标后存储，
 * 供 lv_touch_callback 在下次读取时使用。
 *
 * @param touch 1=按下，0=释放
 * @param x     屏幕物理 X 坐标
 * @param y     屏幕物理 Y 坐标
 */
void LVApp::onTouch(int touch, int x, int y) {
    if (screen_width <= 0 || screen_height <= 0) {
        return;
    }
    isTouch = touch;
    // 坐标映射：屏幕物理坐标 → LVGL 逻辑坐标
    touchX = x * app_width / screen_width;
    touchY = y * app_height / screen_height;
}

// 静态刷新回调包装：通过 user_data 获取实例后调用实例方法
void LVApp::lv_flush_cb_static(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    auto *app = (LVApp *) lv_display_get_user_data(disp);
    app->lv_flush_callback(disp, area, px_map);
}

// 静态触摸回调包装：通过 user_data 获取实例后调用实例方法
void LVApp::lv_touch_cb_static(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    auto *app = (LVApp *) lv_indev_get_user_data(indev_drv);
    app->lv_touch_callback(indev_drv, data);
}

/**
 * 设置要运行的 LVGL Demo 名称。
 * 该名称需与 AppList.h 中的映射 key 一致。
 */
void LVApp::setApp(const char *name) {
    this->app_name = name;
}

/**
 * 设置 LVGL 逻辑分辨率（画布尺寸）。
 * 必须在 start() 之前调用，渲染过程中修改不会生效。
 */
void LVApp::setSize(int w, int h) {
    app_width = w;
    app_height = h;
}

/**
 * 析构函数，确保渲染线程已停止并释放所有资源。
 */
LVApp::~LVApp() {
    stop();
    LOGI("LVApp::~LVApp()!!");
}
