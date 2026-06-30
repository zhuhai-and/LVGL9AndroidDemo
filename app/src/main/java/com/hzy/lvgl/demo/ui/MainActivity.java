package com.hzy.lvgl.demo.ui;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;

import com.hzy.lvgl.demo.databinding.ActivityMainBinding;
import com.hzy.lvgl.demo.model.DemoEntry;
import com.hzy.lvgl.demo.ui.adapter.DemoAdapter;

import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.ArrayList;
import java.util.List;

/**
 * 主页，展示可用的 LVGL Demo 列表。
 * 点击列表项后跳转到 PartScreenActivity 展示对应 Demo。
 */
public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding mB;
    private final List<DemoEntry> mDemoList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mB = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(mB.getRoot());
        setSupportActionBar(mB.mainToolbar);
        ViewCompat.setOnApplyWindowInsetsListener(mB.getRoot(), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        initData();
        initView();
    }

    /**
     * 初始化 Demo 列表数据。
     * 每条记录包含标题、名称、分辨率和屏幕方向。
     * name 字段需与 AppList.h 中的 key 保持一致。
     */
    private void initData() {
        mDemoList.add(new DemoEntry("Demo Widgets", "widgets", 480, 320, ActivityInfo.SCREEN_ORIENTATION_PORTRAIT));
        mDemoList.add(new DemoEntry("Widgets Portrait", "widgets", 320, 480, ActivityInfo.SCREEN_ORIENTATION_PORTRAIT));
        mDemoList.add(new DemoEntry("Chinese Chess", "chess", 320, 480, ActivityInfo.SCREEN_ORIENTATION_PORTRAIT));
        mDemoList.add(new DemoEntry("Demo Music", "music", 480, 320, ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE));
        mDemoList.add(new DemoEntry("Demo Benchmark", "benchmark", 480, 320, ActivityInfo.SCREEN_ORIENTATION_PORTRAIT));
    }

    private void initView() {
        DemoAdapter adapter = new DemoAdapter(mDemoList, this::openPartPage);
        mB.rvDemoList.setLayoutManager(new LinearLayoutManager(this));
        mB.rvDemoList.setAdapter(adapter);
    }

    /**
     * 打开 Demo 展示页，将 DemoEntry 的参数通过 Intent 传递。
     */
    private void openPartPage(DemoEntry entry) {
        Intent intent = new Intent(this, PartScreenActivity.class);
        intent.putExtra(PartScreenActivity.EXTRA_APP, entry.getName());
        intent.putExtra(PartScreenActivity.EXTRA_TITLE, entry.getTitle());
        intent.putExtra(PartScreenActivity.EXTRA_WIDTH, entry.getWidth());
        intent.putExtra(PartScreenActivity.EXTRA_HEIGHT, entry.getHeight());
        intent.putExtra(PartScreenActivity.EXTRA_ORIENTATION, entry.getScreenOrientation());
        startActivity(intent);
    }
}
