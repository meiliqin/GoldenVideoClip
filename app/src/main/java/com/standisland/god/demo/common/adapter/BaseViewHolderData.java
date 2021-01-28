package com.standisland.god.demo.common.adapter;

/**
 * Created by hzsunyj on 2017/12/13.
 */

public class BaseViewHolderData {

    public static final int DEFAULT_VIEW_TYPE = -1;

    private int viewType;

    private Object data;

    public BaseViewHolderData() {
        viewType = DEFAULT_VIEW_TYPE;
    }

    public BaseViewHolderData(int viewType) {
        this.viewType = viewType;
    }

    public BaseViewHolderData(int viewType, Object data) {
        this.viewType = viewType;
        this.data = data;
    }

    public int getViewType() {
        return viewType;
    }

    public void setViewType(int viewType) {
        this.viewType = viewType;
    }

    public Object getData() {
        return data;
    }

    public void setData(Object data) {
        this.data = data;
    }
}
