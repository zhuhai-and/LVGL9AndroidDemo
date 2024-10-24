#ifndef LVGLPORTANDROID_LVAPP_H
#define LVGLPORTANDROID_LVAPP_H

#include <jni.h>
#include <thread>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <lvgl.h>
#include "AppList.h"

class LVApp {
private:
    ANativeWindow *window = nullptr;
    ANativeWindow_Buffer windowBuffer;
    int app_width = 480, app_height = 320;
    int screen_width = 0, screen_height = 0;
    uint16_t *surface_buf = nullptr;
    size_t surface_size = 0;
    string app_name;
    atomic<int> isTouch, touchX, touchY;
    atomic<bool> is_running = false;

    void lv_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

    void lv_touch_callback(lv_indev_t *indev_drv, lv_indev_data_t *data);

    void lv_loop_task();

    static uint32_t lv_tick_get();

    static void lv_log_print(lv_log_level_t level, const char *buf);

    static void lv_flush_cb_static(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

    static void lv_touch_cb_static(lv_indev_t *indev_drv, lv_indev_data_t *data);

public:
    void start(ANativeWindow *window);

    void setApp(const char *name);

    void onTouch(int touch, int x, int y);

    void setSize(int w, int h);

    void stop();
};


#endif //LVGLPORTANDROID_LVAPP_H
