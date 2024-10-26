#include <thread>
#include <mutex>
#include "LVApp.h"
#include "LVHelper.h"
#include <lv_demos.h>
#include <unistd.h>

using namespace std;

void LVApp::lv_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    if (is_running) {
        if (surface_buf == nullptr) {
            surface_size = sizeof(uint16_t) * windowBuffer.stride * windowBuffer.height;
            surface_buf = (uint16_t *) malloc(surface_size);
        }
        int w = area->x2 - area->x1 + 1;
        int h = area->y2 - area->y1 + 1;
        auto *src = (uint16_t *) px_map;
        auto stride = windowBuffer.stride;
        for (int i = 0; i < h; i++) {
            auto *dst = &surface_buf[(area->y1 + i) * stride + area->x1];
            memcpy(dst, &src[i * w], sizeof(uint16_t) * w);
        }
        if (ANativeWindow_lock(window, &windowBuffer, nullptr) == 0) {
            auto *dst = (uint32_t *) windowBuffer.bits;
            memcpy(dst, surface_buf, surface_size);
            ANativeWindow_unlockAndPost(window);
        }
    }
    lv_disp_flush_ready(disp);
}

void LVApp::lv_touch_callback(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    if (isTouch) {
        data->point.x = (short) touchX;
        data->point.y = (short) touchY;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

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

uint32_t LVApp::lv_tick_get() {
    static struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void LVApp::start(ANativeWindow *win) {
    this->window = win;
    screen_width = ANativeWindow_getWidth(window);
    screen_height = ANativeWindow_getHeight(window);
    ANativeWindow_setBuffersGeometry(window, app_width, app_height, WINDOW_FORMAT_RGB_565);
    LOGD("LV Screen [%d x %d]", app_width, app_height);
    is_running = true;
    thread lv_thread(&LVApp::lv_loop_task, this);
    lv_thread.detach();
}

void LVApp::lv_loop_task() {
    LOGD("LV Task Start!!");
    lv_init();
    lv_tick_set_cb(lv_tick_get);
    lv_log_register_print_cb(lv_log_print);

    auto *disp = lv_display_create(app_width, app_height);
    lv_display_set_user_data(disp, this);
    lv_display_set_flush_cb(disp, LVApp::lv_flush_cb_static);

    size_t buf_size = 10 * app_width * app_height;
    auto *buf = malloc(buf_size);
    lv_display_set_buffers(disp, buf, nullptr, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    auto *indev = lv_indev_create();
    lv_indev_set_user_data(indev, this);
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, LVApp::lv_touch_cb_static);

    auto *lv_app_func = lv_ci_example_list.at("default");
    auto search = lv_ci_example_list.find(app_name);
    if (search != lv_ci_example_list.end()) {
        lv_app_func = search->second;
    }
    lv_app_func();
    ANativeWindow_acquire(window);
    if (ANativeWindow_lock(window, &windowBuffer, nullptr) == 0) {
        ANativeWindow_unlockAndPost(window);
    }
    while (is_running) {
        lv_timer_handler();
        usleep(1000);
    }
    ANativeWindow_release(window);
    lv_deinit();
    free(buf);
    if (surface_buf != nullptr) {
        free(surface_buf);
        surface_buf = nullptr;
    }
    window = nullptr;
    LOGD("LV App Stopped!!");
    delete this;
}

void LVApp::stop() {
    is_running = false;
}

void LVApp::onTouch(int touch, int x, int y) {
    isTouch = touch;
    touchX = x * app_width / screen_width;
    touchY = y * app_height / screen_height;
}

void LVApp::lv_flush_cb_static(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    auto *app = (LVApp *) lv_display_get_user_data(disp);
    app->lv_flush_callback(disp, area, px_map);
}

void LVApp::lv_touch_cb_static(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    auto *app = (LVApp *) lv_indev_get_user_data(indev_drv);
    app->lv_touch_callback(indev_drv, data);
}

void LVApp::setApp(const char *name) {
    this->app_name = name;
}

void LVApp::setSize(int w, int h) {
    app_width = w;
    app_height = h;
}

LVApp::~LVApp() {
    LOGI("LVApp::~LVApp()!!");
}
