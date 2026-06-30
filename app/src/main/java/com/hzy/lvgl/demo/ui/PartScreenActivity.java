package com.hzy.lvgl.demo.ui;

import android.annotation.SuppressLint;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintSet;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.hzy.lvgl.demo.databinding.ActivityPartScreenBinding;

/**
 * Demo 展示页，承载 LVGLView 并按 Demo 要求的宽高比约束显示区域。
 *
 * 接收来自 MainActivity 的 Intent 参数：
 *   - app: Demo 名称（对应 AppList.h 中的 key）
 *   - title: 显示标题
 *   - width/height: LVGL 逻辑分辨率
 *   - orientation: 屏幕方向
 */
public class PartScreenActivity extends AppCompatActivity {

    /** Intent extras 键常量，避免拼写错误 */
    public static final String EXTRA_APP = "app";
    public static final String EXTRA_TITLE = "title";
    public static final String EXTRA_WIDTH = "width";
    public static final String EXTRA_HEIGHT = "height";
    public static final String EXTRA_ORIENTATION = "orientation";

    private ActivityPartScreenBinding mB;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mB = ActivityPartScreenBinding.inflate(getLayoutInflater());
        setContentView(mB.getRoot());
        setSupportActionBar(mB.partToolbar);
        ViewCompat.setOnApplyWindowInsetsListener(mB.getRoot(), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        }

        String name = getIntent().getStringExtra(EXTRA_APP);
        int orientation = getIntent().getIntExtra(EXTRA_ORIENTATION, ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        if (orientation != ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED) {
            setRequestedOrientation(orientation);
        }
        if (name != null) {
            int width = getIntent().getIntExtra(EXTRA_WIDTH, 0);
            int height = getIntent().getIntExtra(EXTRA_HEIGHT, 0);
            if (width > 0 && height > 0) {
                // 使用 ConstraintSet 动态设置 FrameLayout 的宽高比，
                // 使 LVGL 画布在保持比例的同时填满可用空间
                ConstraintSet constraintSet = new ConstraintSet();
                constraintSet.clone(mB.lvRoot);
                @SuppressLint("DefaultLocale")
                String ratio = String.format("%d:%d", width, height);
                constraintSet.setDimensionRatio(mB.lvFrame.getId(), ratio);
                constraintSet.applyTo(mB.lvRoot);
                // 先设置 Native 画布尺寸，再选择 Demo，确保 LVGL 初始化时拿到正确分辨率。
                mB.lvView.setSize(width, height);
            }
            mB.lvView.setApp(name);
            setTitle(getIntent().getStringExtra(EXTRA_TITLE));
        }
    }

    @Override
    public boolean onOptionsItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
