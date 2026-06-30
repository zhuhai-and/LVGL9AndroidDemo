package com.hzy.lvgl.demo;

import android.view.Surface;

public class LVApp {

    private long mInstanceId;

    /**
     * LVGL 的全局状态不适合同一进程内并行运行多个主循环。
     * 当前 Java 对象只持有一个 Native 实例，并在 View 彻底销毁时释放。
     */
    public LVApp() {
        mInstanceId = nCreate();
    }

    public long start(Surface surface) {
        if (mInstanceId == 0 || surface == null || !surface.isValid()) {
            return -1;
        }
        return nStart(mInstanceId, surface);
    }

    public long setApp(String name) {
        if (mInstanceId == 0 || name == null) {
            return -1;
        }
        return nSetApp(mInstanceId, name);
    }

    public long setSize(int width, int height) {
        if (mInstanceId == 0 || width <= 0 || height <= 0) {
            return -1;
        }
        return nSetSize(mInstanceId, width, height);
    }

    public long onTouch(int touch, int x, int y) {
        if (mInstanceId == 0) {
            return -1;
        }
        return nOnTouch(mInstanceId, touch, x, y);
    }

    public long stop() {
        if (mInstanceId == 0) {
            return -1;
        }
        return nStop(mInstanceId);
    }

    public void destroy() {
        if (mInstanceId == 0) {
            return;
        }
        nDestroy(mInstanceId);
        mInstanceId = 0;
    }

    private static native long nCreate();

    private static native long nSetApp(long id, String name);

    private static native long nStart(long id, Surface surface);

    private static native long nOnTouch(long id, int touch, int x, int y);

    private static native long nSetSize(long id, int w, int h);

    private static native long nStop(long id);

    private static native long nDestroy(long id);

    static {
        System.loadLibrary("lvApp");
    }
}
