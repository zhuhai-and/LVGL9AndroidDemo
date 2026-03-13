package com.hzy.lvgl.demo.ui;

import android.annotation.SuppressLint;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintSet;

import com.hzy.lvgl.demo.databinding.ActivityPartScreenBinding;

public class PartScreenActivity extends AppCompatActivity {
    private ActivityPartScreenBinding mB;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mB = ActivityPartScreenBinding.inflate(getLayoutInflater());
        setContentView(mB.getRoot());

        if (getSupportActionBar() != null) {
            getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        }

        String name = getIntent().getStringExtra("app");
        int orientation = getIntent().getIntExtra("orientation", ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        if (orientation != ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED) {
            setRequestedOrientation(orientation);
        }
        if (name != null) {
            int width = getIntent().getIntExtra("width", 0);
            int height = getIntent().getIntExtra("height", 0);
            ConstraintSet constraintSet = new ConstraintSet();
            constraintSet.clone(mB.lvRoot);
            @SuppressLint("DefaultLocale")
            String ratio = String.format("%d:%d", width, height);
            constraintSet.setDimensionRatio(mB.lvFrame.getId(), ratio);
            constraintSet.applyTo(mB.lvRoot);
            mB.lvView.setApp(name);
            mB.lvView.setSize(width, height);
            setTitle(name);
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
