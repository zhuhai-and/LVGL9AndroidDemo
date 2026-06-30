#ifndef LVGLPORTANDROID_APPLIST_H
#define LVGLPORTANDROID_APPLIST_H

#include <iostream>
#include <map>
#include <lv_demos.h>
#include "chess/LvChess.h"

using namespace std;

/**
 * LVGL Demo 名称到启动函数的映射表。
 *
 * Java 层通过 LVApp.setApp(name) 传入 key，
 * C++ 层在 LVApp::lv_loop_task() 中用此表查找对应的启动函数。
 *
 * 添加新 Demo 时，只需在此表中新增一行映射即可。
 * key 需要与 Java 层 DemoEntry.name 字段保持一致。
 */
const map<string, void (*)(void)> lv_ci_example_list{
        {"default",   lv_demo_widgets},   // 默认 Demo（Widgets）
        {"benchmark", lv_demo_benchmark}, // 性能基准测试
        {"widgets",   lv_demo_widgets},   // 官方 Widgets 展示
        {"music",     lv_demo_music},     // 官方音乐播放器 Demo
        {"chess",     lv_chess_start},    // 中国象棋人机对弈
};

#endif //LVGLPORTANDROID_APPLIST_H
