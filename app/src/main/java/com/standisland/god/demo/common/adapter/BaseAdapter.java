package com.standisland.god.demo.common.adapter;

import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;

import androidx.recyclerview.widget.RecyclerView;

import java.util.Collections;
import java.util.List;



/**
 * Created by hzsunyj on 16/12/30.
 */
public class BaseAdapter<T> extends RecyclerView.Adapter<BaseViewHolder> {
    private final String TAG = getClass().getName();
    /**
     * data source
     */
    public List<T> dataList;

    /**
     * onClick onLongClick callback
     */
    public OnItemClickListener listener;

    /**
     * constructor view holder delegate
     */
    private BaseDelegate delegate;

    /**
     * constructor
     *
     * @param dataList
     */
    public BaseAdapter(List<T> dataList) {
        this(dataList, null);
    }

    /**
     * constructor
     *
     * @param dataList
     * @param listener
     */
    public BaseAdapter(List<T> dataList, OnItemClickListener listener) {
        checkData(dataList);
        this.listener = listener;
    }

    /**
     * just is empty
     *
     * @param delegate
     */
    public void setDelegate(BaseDelegate delegate) {
        this.delegate = delegate;
    }

    /**
     * just is empty
     *
     * @param dataList
     */
    private void checkData(List<T> dataList) {
        if (dataList == null) {
            dataList = Collections.emptyList();
        }
        this.dataList = dataList;
    }

    /**
     * set onclick & onLongClick callback
     *
     * @param listener
     */
    public void setOnItemClickListener(OnItemClickListener listener) {
        this.listener = listener;
    }

    /**
     * create view holder
     *
     * @param parent
     * @param viewType
     * @return
     */
    @Override
    public BaseViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        BaseViewHolder viewHolder = delegate.createViewHolder(this, parent, viewType);
        if (viewHolder != null) {
            viewHolder.findViews();
        }
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(BaseViewHolder holder, int position, List<Object> payloads) {
        if (dataList == null || dataList.isEmpty()) {
            onBindViewHolder(holder, position);
        } else {
            holder.bindViewHolder(dataList.get(position), payloads);
            listenClick(holder);
        }
    }

    @Override
    public void onBindViewHolder(BaseViewHolder holder, int position) {
        if (holder == null) {
            return;
        }
        holder.bindViewHolder(dataList.get(position), Collections.EMPTY_LIST);
        listenClick(holder);
    }


    protected void listenClick(BaseViewHolder holder) {
        if (listener != null && holder.enable()) {
            holder.itemView.setOnClickListener(mClickListenerMediator);
            holder.itemView.setOnLongClickListener(mLongClickListenerMediator);
        }
    }


    public void updateDataAndNotify(List<? extends T> list) {
        dataList.clear();
        if (list != null && list.size() > 0) {
            dataList.addAll(list);
        }
        this.notifyDataSetChanged();
    }

    public void appendDataAndNotify(T t) {
        if (t == null) {
            return;
        }
        dataList.add(t);
        this.notifyDataSetChanged();
    }

    public void insertHeadDataAndNotify(T t) {
        if (t == null) {
            return;
        }
        dataList.add(0, t);
        this.notifyDataSetChanged();
    }

    public void insertDataAndNotify(T t, int index) {
        if (t == null) {
            return;
        }
        dataList.add(index, t);
        this.notifyDataSetChanged();
    }


    public void appendDataAndNotify(List<? extends T> list) {
        dataList.addAll(list);
        this.notifyDataSetChanged();
    }

    public void insertDataAndNotify(List<? extends T> list, int index) {
        dataList.addAll(index, list);
        this.notifyDataSetChanged();
    }

    public void resetDataList(List<T> list) {
        checkData(list);
//        this.dataList = list;
    }

    public boolean isEmpty() {
        return dataList.size() == 0;
    }

    /**
     * get item count
     *
     * @return
     */
    @Override
    public int getItemCount() {
        return dataList == null ? 0 : dataList.size();
    }

    /**
     * get item view type
     *
     * @param position
     * @return
     */
    @Override
    public int getItemViewType(int position) {
        return delegate.getItemViewType(dataList.get(position), position);
    }

    private View.OnClickListener mClickListenerMediator = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if (listener != null) {
                int pos = getViewHolderAdapterPosition(v);
                if(pos<0 || pos > getItemCount()-1){
                    return;
                }
                listener.onClick(v, pos, getData(pos));
            }
        }
    };

    private View.OnLongClickListener mLongClickListenerMediator = new View.OnLongClickListener() {
        @Override
        public boolean onLongClick(View v) {
            if (listener != null) {
                int pos = getViewHolderAdapterPosition(v);
                if(pos<0 || pos > getItemCount()-1){
                    return false;
                }
                return listener.onLongClick(v, pos, getData(pos));
            }
            return false;
        }
    };

    public T getData(int pos) {
        return pos >= 0 ? dataList.get(pos) : null;
    }

    static int getViewHolderAdapterPosition(View v) {
        if (v != null) {
            ViewParent parent = v.getParent();
            if (parent instanceof RecyclerView) {
                return ((RecyclerView) parent).getChildAdapterPosition(v);
            }
        }
        return -1;
    }

}
