package com.hzy.lvgl.demo;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

/**
 * LVGL 渲染视图，继承自 SurfaceView。
 *
 * 职责：
 *   - 承载 LVGL 的 Native 渲染输出（通过 Surface → ANativeWindow）
 *   - 将 Android 触摸事件转发给 Native 层
 *   - 管理 LVGL 实例的生命周期（创建、暂停、恢复、销毁）
 *
 * 生命周期说明：
 *   - 构造时创建 LVApp 实例（Native 对象）
 *   - surfaceChanged 时启动渲染（start）
 *   - surfaceDestroyed 时暂停渲染（stop），但保留 Native 实例
 *   - onDetachedFromWindow 时销毁 Native 实例（destroy）
 */
public class LVGLView extends SurfaceView implements SurfaceHolder.Callback {

    private final LVApp mLVApp;
    private boolean mDestroyed;  // 标记 Native 实例是否已销毁，防止重复调用

    public LVGLView(Context context) {
        this(context, null);
    }

    public LVGLView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public LVGLView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        getHolder().addCallback(this);
        setFocusable(true);
        setKeepScreenOn(true);
        setFocusableInTouchMode(true);
        mLVApp = new LVApp();
    }

    /**
     * 设置要运行的 LVGL Demo 名称。
     * 需在 Surface 创建前调用，名称对应 AppList.h 中的 key。
     */
    public void setApp(String name) {
        if (mDestroyed) {
            return;
        }
        mLVApp.setApp(name);
    }

    /**
     * 设置 LVGL 逻辑分辨率（画布尺寸）。
     * 需在 Surface 创建前调用，影响 Native 层 NativeWindow 的缓冲区大小。
     */
    public void setSize(int width, int height) {
        if (mDestroyed) {
            return;
        }
        mLVApp.setSize(width, height);
    }

    /**
     * 触摸事件处理，将按下/移动/抬起事件转发给 Native 层。
     * touch=1 表示按下或移动，touch=0 表示抬起或取消。
     */
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mDestroyed) {
            return false;
        }
        int x = (int) event.getX();
        int y = (int) event.getY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_MOVE:
                mLVApp.onTouch(1, x, y);  // 按下/移动：传递状态 1
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                mLVApp.onTouch(0, x, y);  // 抬起/取消：传递状态 0
                break;
        }
        return super.onTouchEvent(event);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
        // Surface 创建时不启动渲染，等待 surfaceChanged 确认尺寸后再启动
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // Surface 尺寸确定后启动 LVGL 渲染
        Surface surface = holder.getSurface();
        if (!mDestroyed && surface != null && surface.isValid()) {
            mLVApp.start(surface);
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {
        // Surface 销毁只暂停渲染，不销毁 Native 实例（便于恢复时快速重启）
        mLVApp.stop();
    }

    @Override
    protected void onDetachedFromWindow() {
        // Surface 销毁只代表暂停渲染；View 离开窗口时才释放 Native LVGL 实例。
        if (!mDestroyed) {
            mLVApp.destroy();
            mDestroyed = true;
        }
        super.onDetachedFromWindow();
    }
}
