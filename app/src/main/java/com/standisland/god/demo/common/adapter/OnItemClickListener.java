package com.standisland.god.demo.common.adapter;

import android.view.View;

/**
 * Created by hzsunyj on 16/12/30.
 */
public interface OnItemClickListener<T> {

    void onClick(View v, int pos, T data);

    boolean onLongClick(View v, int pos, T data);
}
