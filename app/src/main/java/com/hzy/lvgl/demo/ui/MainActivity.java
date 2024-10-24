package com.hzy.lvgl.demo.ui;

import android.content.Intent;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.blankj.utilcode.util.ActivityUtils;
import com.hzy.lvgl.demo.databinding.ActivityMainBinding;


public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding mB;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mB = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(mB.getRoot());
        mB.btnWidgets.setOnClickListener(v -> openPartPage("widgets"));
        mB.btnMusic.setOnClickListener(v -> openPartPage("music"));
        mB.btnBenchmark.setOnClickListener(v -> openPartPage("benchmark"));
        mB.btnLarge.setOnClickListener(v -> ActivityUtils.startActivity(FullScreenActivity.class));
    }

    private void openPartPage(String name) {
        openPartPage(name, 480, 320);
    }

    private void openPartPage(String name, int width, int height) {
        Intent intent = new Intent(this, PartScreenActivity.class);
        intent.putExtra("app", name);
        intent.putExtra("width", width);
        intent.putExtra("height", height);
        startActivity(intent);
    }
}
