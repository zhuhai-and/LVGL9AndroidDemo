package com.hzy.lvgl.demo.ui.adapter;

import android.view.LayoutInflater;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.hzy.lvgl.demo.databinding.ItemDemoEntryBinding;
import com.hzy.lvgl.demo.model.DemoEntry;

import java.util.List;

/**
 * Demo 列表的 RecyclerView 适配器。
 * 每个列表项显示 Demo 标题和分辨率描述，点击时触发回调。
 */
public class DemoAdapter extends RecyclerView.Adapter<DemoAdapter.DemoViewHolder> {

    private final List<DemoEntry> data;
    private final OnItemClickListener listener;

    /** 列表项点击回调接口 */
    public interface OnItemClickListener {
        void onItemClick(DemoEntry entry);
    }

    public DemoAdapter(List<DemoEntry> data, OnItemClickListener listener) {
        this.data = data;
        this.listener = listener;
    }

    @NonNull
    @Override
    public DemoViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        ItemDemoEntryBinding binding = ItemDemoEntryBinding.inflate(
                LayoutInflater.from(parent.getContext()), parent, false);
        return new DemoViewHolder(binding);
    }

    @Override
    public void onBindViewHolder(@NonNull DemoViewHolder holder, int position) {
        holder.bind(data.get(position), listener);
    }

    @Override
    public int getItemCount() {
        return data.size();
    }

    static class DemoViewHolder extends RecyclerView.ViewHolder {
        private final ItemDemoEntryBinding binding;

        public DemoViewHolder(@NonNull ItemDemoEntryBinding binding) {
            super(binding.getRoot());
            this.binding = binding;
        }

        public void bind(DemoEntry entry, OnItemClickListener listener) {
            binding.tvTitle.setText(entry.getTitle());
            String desc = entry.getWidth() + "x" + entry.getHeight();
            binding.tvDesc.setText(desc);
            binding.getRoot().setOnClickListener(v -> listener.onItemClick(entry));
        }
    }
}
