package com.standisland.god.demo.feature.sticker.IMG;

import android.content.Context;
import android.graphics.Color;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.standisland.god.demo.R;
import com.standisland.god.demo.common.utils.FontUtils;
import com.standisland.god.demo.feature.sticker.TextEditCallback;

import java.util.Objects;


/**
 * Created by felix on 2017/11/14 下午7:27.
 */
public class IMGStickerTextView extends IMGStickerView implements TextEditCallback {

    private static final String TAG = "IMGStickerTextView";

    private View mContentView;

    private TextView mTextView;

    private IMGText mText;

    private TextClickListener mTextClickListener;

    private static float mBaseTextSize = -1f;

    private static final int PADDING = 24;

    private static final float TEXT_SIZE_SP = 26f;

    public IMGStickerTextView(Context context) {
        this(context, null, 0);
    }

    public IMGStickerTextView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public IMGStickerTextView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public void onInitialize(Context context) {
        super.onInitialize(context);
    }

    @Override
    public View onCreateContentView(Context context, ViewGroup viewGroup) {
        mContentView = LayoutInflater.from(context).inflate(R.layout.image_widget_text_sticker_layout, viewGroup, false);
        mTextView = mContentView.findViewById(R.id.tv_sticker);
        return mContentView;
    }


    public TextView getTextView() {
        return mTextView;
    }

    public void setText(IMGText text) {
        if (text == null || mTextView == null) {
            return;
        }
        mText = text;
        int color = mText.getColor();
        if (mText.isTextBg()) {
            mTextView.setBackgroundColor(color);
            if (color == Color.WHITE) {
                mTextView.setTextColor(Color.BLACK);
            } else {
                mTextView.setTextColor(Color.WHITE);
            }
        } else {
            mTextView.setBackground(null);
            mTextView.setTextColor(color);
        }
        try {
            String fontPath = mText.getFontPath();
            if (!TextUtils.isEmpty(fontPath)) {
                FontUtils.getInstance().replaceFontFromFile(mTextView, fontPath);
            }
            mTextView.setText(mText.getText());
        } catch (Exception e) {
            e.printStackTrace();
        }
        mContentView.setVisibility(isVideoMode() ? INVISIBLE : VISIBLE);
    }

    public IMGText getText() {
        return mText;
    }

    @Override
    public void onContentTap() {
        if (mTextClickListener != null) {
            mTextClickListener.onTextClick(mText, this);
        }
    }

    @Override
    public void onText(IMGText imgText) {
        boolean update = !Objects.equals(imgText, mText);
        if (update) {
            if (mText == null) {
                setText(imgText);
            } else {
                mText.set(imgText);
                setText(mText);
            }
            mStickerHelper.onContentUpdate(this);
        }
    }

    public void setTextClickListener(TextClickListener textClickListener) {
        mTextClickListener = textClickListener;
    }

    public interface TextClickListener {

        void onTextClick(IMGText imgText, IMGStickerTextView stickerTextView);

    }


}