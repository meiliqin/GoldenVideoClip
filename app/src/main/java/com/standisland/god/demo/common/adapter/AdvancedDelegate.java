package com.standisland.god.demo.common.adapter;

/**
 * Created by hzqiujiadi on 2017/12/25.
 * hzqiujiadi ashqalcn@gmail.com
 */

public abstract class AdvancedDelegate extends BaseDelegate<BaseViewHolderData> {

    private static final int sTypeBase = 569100;
    public static final int sTypeEmpty = sTypeBase + 1;
    protected static final int sTypeLoading = sTypeBase + 2;
    public static final int sTypeLoadingMore = sTypeBase + 3;
    protected static final int sTypeToolbarHeight = sTypeBase + 4;
    protected static final int sTypeVideoBottom = sTypeBase + 5;

    @Override
    public int getItemViewType(BaseViewHolderData data, int pos) {
        return data.getViewType();
    }

}
