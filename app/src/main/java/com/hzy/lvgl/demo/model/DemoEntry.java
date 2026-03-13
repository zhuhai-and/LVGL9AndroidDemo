package com.hzy.lvgl.demo.model;

import android.content.pm.ActivityInfo;

public class DemoEntry {
    private String name;
    private String title;
    private int width;
    private int height;
    private int screenOrientation;

    public DemoEntry(String title, String name, int width, int height, int screenOrientation) {
        this.title = title;
        this.name = name;
        this.width = width;
        this.height = height;
        this.screenOrientation = screenOrientation;
    }

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
