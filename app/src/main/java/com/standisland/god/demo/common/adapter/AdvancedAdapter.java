package com.standisland.god.demo.common.adapter;

import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Created by hzqiujiadi on 2017/12/25.
 * hzqiujiadi ashqalcn@gmail.com
 */

public class AdvancedAdapter extends BaseAdapter<BaseViewHolderData> {
    protected AdvancedDelegate specialDelegate;

    public AdvancedAdapter() {
        super(new ArrayList<BaseViewHolderData>());
    }

    public AdvancedAdapter(OnItemClickListener listener) {
        super(new ArrayList<BaseViewHolderData>(), listener);
    }


    @Override
    public void onBindViewHolder(BaseViewHolder holder, int position, List<Object> payloads) {
        holder.bindViewHolder(this, dataList.get(position).getData(), payloads);
        listenClick(holder);
        hackBindViewHolder(holder, position, payloads);
    }

    @Override
    public void onBindViewHolder(BaseViewHolder holder, int position) {
        super.onBindViewHolder(holder, position);
        hackBindViewHolder(holder, position, null);
    }

    protected void hackBindViewHolder(BaseViewHolder holder, int position, List<Object> payloads) {
        //子类可选择实现
    }

    public void clear() {
        this.dataList.clear();
    }

    public void add(int type, Object data) {
        this.dataList.add(new BaseViewHolderData(type, data));
    }

    public void add(int type, Object data, int position) {
        this.dataList.add(position, new BaseViewHolderData(type, data));
    }

    @Override
    public void setDelegate(final BaseDelegate delegate) {
        super.setDelegate(new AdvancedDelegate() {
            @Override
            public BaseViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
                if (specialDelegate != null) {
                    BaseViewHolder vh = specialDelegate.onCreateViewHolder(parent, viewType);
                    if (vh != null) {
                        return vh;
                    }
                }
                return delegate.onCreateViewHolder(parent, viewType);
            }
        });
    }

    @Override
    public final int getItemCount() {
        return super.getItemCount();
    }

    @Override
    public final int getItemViewType(int position) {
        return super.getItemViewType(position);
    }

    public void removeByTypeAll(int type) {
        remove(type, false);
    }

    public void removeByType(int type) {
        remove(type, true);
    }

    private void remove(int type, boolean onlyOne) {
        Iterator<BaseViewHolderData> iterator = this.dataList.iterator();
        while (iterator.hasNext()) {
            BaseViewHolderData data = iterator.next();
            if (data.getViewType() == type) {
                iterator.remove();
                if (onlyOne) {
                    break;
                }
            }
        }
    }


    /**
     * 替换对应类型的布局
     */
    public void setSpecialDelegate(AdvancedDelegate advanceDelegate) {
        this.specialDelegate = advanceDelegate;
    }
}
