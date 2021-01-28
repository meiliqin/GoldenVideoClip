package com.standisland.god.demo.common.adapter;

import android.view.View;

/**
 * Created by hzsunyj on 2018/1/25.
 */
public class DefaultItemClickLister<T> implements OnItemClickListener<T> {

    @Override
    public void onClick(View v, int pos, T data) {

    }

    @Override
    public boolean onLongClick(View v, int pos, T data) {
        return false;
    }
}
