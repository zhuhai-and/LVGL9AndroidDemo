package com.hzy.lvgl.demo.model;

import android.content.pm.ActivityInfo;

/**
 * Demo 列表数据模型。
 *
 * 封装单个 Demo 的展示信息：标题、名称（对应 AppList.h 的 key）、
 * 逻辑分辨率和屏幕方向。
 */
public class DemoEntry {
    private final String name;              // Demo 标识，对应 AppList.h 中的 key
    private final String title;             // 显示标题
    private final int width;                // LVGL 逻辑宽度（像素）
    private final int height;               // LVGL 逻辑高度（像素）
    private final int screenOrientation;    // 屏幕方向（ActivityInfo.SCREEN_ORIENTATION_*）

    /**
     * 全参数构造函数。
     *
     * @param title             显示标题
     * @param name              Demo 标识（需与 AppList.h 的 key 一致）
     * @param width             LVGL 逻辑宽度
     * @param height            LVGL 逻辑高度
     * @param screenOrientation 屏幕方向
     */
    public DemoEntry(String title, String name, int width, int height, int screenOrientation) {
        this.title = title;
        this.name = name;
        this.width = width;
        this.height = height;
        this.screenOrientation = screenOrientation;
    }

    /**
     * 简化构造函数，默认分辨率 480×320，横屏。
     */
    public DemoEntry(String title, String name) {
        this(title, name, 480, 320, ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    public String getName() {
        return name;
    }

    public String getTitle() {
        return title;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public int getScreenOrientation() {
        return screenOrientation;
    }
}
