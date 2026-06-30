package com.hzy.lvgl.demo;

import android.app.Application;

import com.blankj.utilcode.util.Utils;

/**
 * 应用入口，初始化 blankj 工具库。
 */
public class MainApp extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        Utils.init(this);
    }
}
