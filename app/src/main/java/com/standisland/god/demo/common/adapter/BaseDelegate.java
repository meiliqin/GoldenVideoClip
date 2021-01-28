package com.standisland.god.demo.common.adapter;

import android.view.ViewGroup;

/**
 * Created by hzsunyj on 16/12/30.
 */
public abstract class BaseDelegate<T> {

    final BaseViewHolder createViewHolder(BaseAdapter<T> adapter, ViewGroup parent, int viewType) {
        BaseViewHolder vh = onCreateViewHolder(parent, viewType);
        if (vh != null) {
            vh.mAdapter = adapter;
        }
        return vh;
    }

    /**
     * crate view holder by view type
     *
     * @param parent
     * @param viewType
     * @return
     */
    public abstract BaseViewHolder onCreateViewHolder(ViewGroup parent, int viewType);

    /**
     * get view type by data
     *
     * @param data
     * @param pos
     * @return
     */
    public abstract int getItemViewType(T data, int pos);
}
