package com.standisland.god.demo.common.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.recyclerview.widget.RecyclerView;

import java.util.Collections;
import java.util.List;

/**
 * Created by hzsunyj on 16/12/30.
 */
public abstract class BaseViewHolder<T> extends RecyclerView.ViewHolder {

    public T mBindData;
    public T mOldBindData;
    public BaseAdapter<T> mAdapter;
    public Context context;

    public BaseViewHolder(ViewGroup parent, int layoutId) {
        this(LayoutInflater.from(parent.getContext()).inflate(layoutId, parent, false));
    }

    /**
     * single view may be direct construction, eg: TextView view = new TextView(context);
     *
     * @param view
     */
    public BaseViewHolder(View view) {
        super(view);
        context = view.getContext();
    }

    /**
     * find all views
     */
    public abstract void findViews();

    /**
     * bind view holder
     *
     * @param data
     */
    protected abstract void onBindViewHolder(T data, List<Object> payloads);

    /**
     * holder click enable
     *
     * @return
     */
    public boolean enable() {
        return true;
    }

    public boolean isLastItem() {
        return mAdapter.getItemCount() == getAdapterPosition() + 1;
    }

    public T getLastIem() {
        if (mAdapter.getItemCount() > 0) {
            return mAdapter.getData(mAdapter.getItemCount() - 1);
        }
        return null;
    }

    public boolean isFirstItem() {
        return getAdapterPosition() == 0;
    }

    public boolean isFirstItem(T t) {
        if (t != null && mAdapter != null && mAdapter.dataList != null && !mAdapter.dataList.isEmpty()) {
            Object obj = mAdapter.dataList.get(0);
            if (obj != null && obj instanceof BaseViewHolderData) {
                return t == ((BaseViewHolderData) obj).getData();
            }
        }
        return false;
    }

    public BaseAdapter getAdapter() {
        return mAdapter;
    }

    public final void bindViewHolder(T data, List<Object> payloads) {
        this.mOldBindData = mBindData;
        this.mBindData = data;
        onBindViewHolder(data, payloads);
    }

    public final void bindViewHolder(BaseAdapter<T> adapter, T data, List<Object> payloads) {
        this.mAdapter = adapter;
        bindViewHolder(data, payloads);
    }

    public final void bindViewHolder(T data) {
        bindViewHolder(data, Collections.EMPTY_LIST);
    }
}
