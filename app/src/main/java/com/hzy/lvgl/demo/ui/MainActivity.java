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

    private void openPartPage(DemoEntry entry) {
        Intent intent = new Intent(this, PartScreenActivity.class);
        intent.putExtra("app", entry.getName());
        intent.putExtra("title", entry.getTitle());
        intent.putExtra("width", entry.getWidth());
        intent.putExtra("height", entry.getHeight());
        intent.putExtra("orientation", entry.getScreenOrientation());
        startActivity(intent);
    }
}
