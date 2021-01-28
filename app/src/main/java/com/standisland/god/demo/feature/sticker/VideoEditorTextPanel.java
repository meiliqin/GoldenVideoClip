package com.standisland.god.demo.feature.sticker;

import android.content.Context;
import android.graphics.Color;
import android.text.Editable;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;

import androidx.annotation.Nullable;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;


import com.bumptech.glide.Glide;
import com.standisland.god.demo.R;
import com.standisland.god.demo.common.adapter.AdvancedAdapter;
import com.standisland.god.demo.common.adapter.AdvancedDelegate;
import com.standisland.god.demo.common.adapter.BaseViewHolder;
import com.standisland.god.demo.common.adapter.BaseViewHolderData;
import com.standisland.god.demo.common.adapter.OnItemClickListener;
import com.standisland.god.demo.common.utils.DisplayUtil;
import com.standisland.god.demo.common.utils.KeyboardUtil;
import com.standisland.god.demo.common.view.CircleProgressView;
import com.standisland.god.demo.feature.sticker.IMG.IMGText;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;



public class VideoEditorTextPanel extends LinearLayout {
    private RecyclerView rvColor, rvFont;
    private ImageView ivClose, ivOk, ivTextBg;
    private FontEditText etContent;
    private FrameLayout flTextBg;
    private boolean textBg = false;
    private int currentColor =  Color.WHITE;
    private String currentFont = null;
    private DialogVideoEditorTextPanel dialog;
    private String currentFontUrl;
    private TextEditCallback textEditCallback;
    private TextFontAdapter textFontAdapter = new TextFontAdapter();
    private TextColorAdapter textColorAdapter = new TextColorAdapter();
    private int mFromPage;
    private long startTime = 0;

    public VideoEditorTextPanel(Context context) {
        this(context, null);
    }

    public VideoEditorTextPanel(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public VideoEditorTextPanel(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView(context);
    }

    public void setmFromPage(int fromPage) {
        this.mFromPage = fromPage;
        textFontAdapter.setFromPage(mFromPage);
    }

    @Override
    public void setVisibility(int visibility) {
        super.setVisibility(visibility);
        if(visibility==VISIBLE){
            startTime = System.currentTimeMillis();
        }
    }

    private void initView(Context context) {
        LayoutInflater.from(context).inflate(R.layout.view_video_editor_text_panel, this);
        ivClose = findViewById(R.id.iv_sticker_close);
        ivOk = findViewById(R.id.iv_sticker_ok);
        rvColor = findViewById(R.id.rv_color);
        rvFont = findViewById(R.id.rv_font);
        etContent = findViewById(R.id.et_content);
        ivTextBg = findViewById(R.id.iv_editor_text_bg);
        flTextBg = findViewById(R.id.fl_editor_text_bg);


        rvColor.setLayoutManager(new LinearLayoutManager(context, RecyclerView.HORIZONTAL, false));
        rvColor.setAdapter(textColorAdapter);
        List<Integer> data = new ArrayList<>();
        data.add(Color.WHITE);
        data.add(Color.BLACK);
        data.add(Color.parseColor("#FF5858"));
        data.add(Color.parseColor("#FAE94D"));
        data.add(Color.parseColor("#6EE47A"));
        data.add(Color.parseColor("#5EDFE4"));
        data.add(Color.parseColor("#7458FF"));
        textColorAdapter.update(data);
        textColorAdapter.setOnItemClickListener(new OnItemClickListener<BaseViewHolderData>() {
            @Override
            public void onClick(View view, int i, BaseViewHolderData o) {
                if (o != null && o.getData() != null && o.getData() instanceof Integer) {
                    currentColor = (int) o.getData();
                    updateTextColor();
                }
                textColorAdapter.updateSelect(i);
            }

            @Override
            public boolean onLongClick(View view, int i, BaseViewHolderData o) {
                return false;
            }
        });


        rvFont.setLayoutManager(new LinearLayoutManager(context, RecyclerView.HORIZONTAL,false));
        rvFont.setAdapter(textFontAdapter);


        textFontAdapter.setOnItemClickListener(new OnItemClickListener<BaseViewHolderData>() {
            @Override
            public void onClick(View view, int i, BaseViewHolderData o) {
                if (o != null && o.getData() != null && o.getData() instanceof GlAssetEntity) {
                    GlAssetEntity glAssetEntity = (GlAssetEntity) o.getData();
//                    VideoEditorTextDownloader textDownloader = VideoEditorDownloaderManager.getInstance().getTextDownloader();
//                    boolean downloaded = textDownloader.isFileDownloaded(glAssetEntity.getFileName());
                    // 字体未下载完成是，不可选中
                   // if (downloaded) {
                        currentFont = glAssetEntity.getFileName();
                        currentFontUrl = glAssetEntity.getUrl();
                        textFontAdapter.updateSelect(i);
                        updateTextFont();
//                    } else {
//                        textDownloader.download(glAssetEntity.getUrl(), null, glAssetEntity.getFileSize(), glAssetEntity.getFileName(), downloadCallBack);
//                    }
                }
            }

            @Override
            public boolean onLongClick(View view, int i, BaseViewHolderData o) {
                return false;
            }
        });
        textFontAdapter.update();

        ivClose.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (dialog != null) {
                    dialog.dismiss();
                } else {
                    setVisibility(GONE);
                    KeyboardUtil.hideKeyboard(etContent);
                }
            }
        });
        flTextBg.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                textBg = !textBg;
                if (textBg) {
                    ivTextBg.setImageResource(R.drawable.ic_editor_text_bg_no);
                } else {
                    ivTextBg.setImageResource(R.drawable.ic_editor_text_bg_yes);
                }
                updateTextColor();
            }
        });

        ivOk.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (textEditCallback != null) {
                    Editable editable = etContent.getText();
                    String text = editable != null ? editable.toString() : "";
                    if (editable != null) {
                        editable.clear();
                    }
                    IMGText imgText = new IMGText(text, currentColor);
                    imgText.setTextBg(textBg);
                    if (!TextUtils.isEmpty(currentFont)) {
                        imgText.setFontName(currentFont);
//                        imgText.setFontPath(VideoEditorDownloaderManager.getInstance().getTextDownloader().getPath(currentFont));
                    }
                    textEditCallback.onText(imgText);
                }
                if (dialog != null) {
                    dialog.dismiss();
                }

            }
        });
    }

    public FontEditText getEtContent() {
        return etContent;
    }

    public void updateTextFont(){
        if(!TextUtils.isEmpty(currentFont)){
//            etContent.setFontPath(VideoEditorDownloaderManager.getInstance().getTextDownloader().getPath(currentFont));
        }
    }

    public void updateTextColor(){
        if(textBg){
            etContent.setBackgroundColor(currentColor);
            if (currentColor == Color.WHITE) {
                etContent.setTextColor(Color.BLACK);
            } else {
                etContent.setTextColor(Color.WHITE);
            }
        } else {
            etContent.setBackground(null);
            etContent.setTextColor(currentColor);
        }
    }

    public void setTextCallback(TextEditCallback callback) {
        textEditCallback = callback;
    }


    public void setDialog(DialogVideoEditorTextPanel dialog) {
        this.dialog = dialog;
    }

    /**
     * 设置文字内容
     */
    public void setTextSticker(IMGText imgText) {
        if (imgText == null) {
            // TODO Dodge 还原文字
            etContent.setText("");
            updateTextColor();
//            etContent.setTextColor(currentColor);
        } else {
            currentColor = imgText.getColor();
            textBg = imgText.isTextBg();
            currentFont = imgText.getFontName();
            etContent.setText(imgText.getText());
            updateTextFont();
            updateTextColor();
            textColorAdapter.updateSelectColor(currentColor);
            textFontAdapter.updateSelectFont(currentFont);
        }
    }

    private static class TextColorAdapter extends AdvancedAdapter {
        public static final int TYPE_ITEM = 1;
        private int current = 0;
        private List<Integer> data;
        public TextColorAdapter() {
            setDelegate(new AdvancedDelegate() {
                @Override
                public BaseViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
                    switch (i) {
                        case TYPE_ITEM:
                            return new TextColorHolder(LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_editor_text_color, viewGroup, false));
                    }
                    return null;
                }
            });

        }

        public void update(List<Integer> data) {
            this.data = data;
            clear();
            if (data != null) {
                for (Integer integer : data) {
                    add(TYPE_ITEM, integer);
                }
            }
            notifyDataSetChanged();
        }

        public void updateSelectColor(int selectColor){
            if(data!=null){
                int position = data.indexOf(selectColor);
                if(position==-1){
                    position = 0;
                }
                updateSelect(position);
            }
        }

        public void updateSelect(int index) {
            if(current==index){
                return;
            }
            if(current>-1){
                notifyItemChanged(current,index);
            }
            notifyItemChanged(index,index);
            current = index;
        }
    }

    private static class TextColorHolder extends BaseViewHolder<Integer>{
        private int selectIndex = -1;
        private CircleView cvColor;
        private int small,big;
        private boolean init = true;

        public TextColorHolder(View view) {
            super(view);
            small = DisplayUtil.dip2px(view.getContext(),28);
            big = DisplayUtil.dip2px(view.getContext(),32);
        }

        @Override
        public void findViews() {
            cvColor = itemView.findViewById(R.id.cv_color);
        }

        @Override
        protected void onBindViewHolder(Integer integer, List<Object> list) {
            cvColor.setColor(integer);

            if(list!=null && list.size()>0 && list.get(0)!=null && list.get(0) instanceof Integer){
                selectIndex = (int) list.get(0);
            }else if(list.size()==0){
                selectIndex = -1;
            }
            if(init && getAdapterPosition()==0){
                selectIndex = 0;
                init = false;
            }
            ViewGroup.LayoutParams layoutParams = cvColor.getLayoutParams();
            if(getAdapterPosition()==selectIndex){
                layoutParams.width = big;
                layoutParams.height = big;
            }else{
                layoutParams.width = small;
                layoutParams.height = small;
            }
            cvColor.setLayoutParams(layoutParams);

        }
    }

    private static class TextFontHolder extends BaseViewHolder<GlAssetEntity>{
        private ImageView ivFont;
        private CircleProgressView cpFont;
        private ImageView ivFontDownload;
        private boolean init = true;
        private int selectIndex = -1;
        private TextFontAdapter adapter;
        private int fromPage;
        public TextFontHolder(View view, TextFontAdapter adapter, int fromPage) {
            super(view);
            this.adapter = adapter;
            this.fromPage = fromPage;
        }

        @Override
        public void findViews() {
            ivFont = itemView.findViewById(R.id.iv_font);
            cpFont = itemView.findViewById(R.id.progress_loading);
            ivFontDownload = itemView.findViewById(R.id.iv_download);
        }

        @Override
        protected void onBindViewHolder(GlAssetEntity s, List<Object> list) {
            Glide.with(ivFont).load(s.getImage()).into(ivFont);
            if(list!=null && list.size()>0 && list.get(0)!=null && list.get(0) instanceof Integer){
                selectIndex = (int) list.get(0);
            }else if(list.size()==0){
                selectIndex = -1;
            }
            if(getAdapterPosition()==selectIndex){
                ((View)ivFont.getParent()).setSelected(true);
            }else{
                ((View)ivFont.getParent()).setSelected(false);
            }
//            if(VideoEditorDownloaderManager.getInstance().getTextDownloader().isFileDownloaded(s.getFileName())){
//                ivFontDownload.setVisibility(GONE);
//            }else{
//                ivFontDownload.setVisibility(VISIBLE);
//            }
            adapter.registerHolderUrl(s.getUrl(),this);

        }

        public void updateProgress(float progress){
            ivFontDownload.setVisibility(GONE);
            cpFont.setVisibility(VISIBLE);
            cpFont.setProgress(progress);
        }

        public void downloadFail(){
            ivFontDownload.setVisibility(VISIBLE);
            cpFont.setVisibility(GONE);
            cpFont.setProgress(0);
        }

        public void updateSuccess() {
            ivFontDownload.setVisibility(GONE);
            cpFont.setVisibility(GONE);
        }
    }

    private static class TextFontAdapter extends AdvancedAdapter{
        public static final int TYPE_ITEM = 1;
        private int current = -1;

        private Map<String, Float> downloadProgressMap = new HashMap<>();
        private Map<String,TextFontHolder> holderIndex = new HashMap<>();
        private List<GlAssetEntity> data = new ArrayList<>();
        private int fromPage;

        public TextFontAdapter() {
            setDelegate(new AdvancedDelegate() {
                @Override
                public BaseViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
                    switch (i){
                        case TYPE_ITEM:
                            return new TextFontHolder(LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.item_editor_text_font,
                                    viewGroup, false), TextFontAdapter.this, fromPage);
                    }
                    return null;
                }
            });
        }

        public void setFromPage(int fromPage) {
            this.fromPage = fromPage;
        }

        public void update(){
            clear();
            Map<String, GlAssetEntity> map = null;
//            Map<String, GlAssetEntity> map = GlAssetsManager.get().getLocalByType(GlAssetsRepo.EDITOR_TEXT_ASSERT);
            if(map!=null && map.size()>0){
                for (Map.Entry<String,GlAssetEntity> entry:map.entrySet()){
                    add(TYPE_ITEM,entry.getValue());
                    data.add(entry.getValue());
                }
            }
            notifyDataSetChanged();
        }

        public void updateSelectFont(String fontName){
            if(data!=null && !TextUtils.isEmpty(fontName)){
                for (int i = 0; i < data.size(); i++) {
                    GlAssetEntity entity = data.get(i);
                    if(entity!=null && entity.getFileName()!=null && entity.getFileName().equals(fontName)){
                        updateSelect(i);
                        break;
                    }
                }

            }
        }


        public void updateSelect(int index) {
            if(current==index){
                return;
            }
            if(current>-1){
                notifyItemChanged(current,index);
            }
            notifyItemChanged(index,index);
            current = index;
        }

        public void updateProgress(String url, float progress){
            Log.d("updateProgress",progress+"");
            downloadProgressMap.put(url,progress);
            if(holderIndex.get(url)!=null){
                holderIndex.get(url).updateProgress(progress * 100);
            }
        }

        public void updateFail(String url) {
            downloadProgressMap.remove(url);
            if(holderIndex.get(url)!=null){
                holderIndex.get(url).downloadFail();
            }
        }

        public void registerHolderUrl(String url, TextFontHolder holder) {
            holderIndex.put(url,holder);
        }

        public void updateSuccess(String url) {
            downloadProgressMap.remove(url);
            if(holderIndex.get(url)!=null){
                holderIndex.get(url).updateSuccess();
            }
        }
    }


}
