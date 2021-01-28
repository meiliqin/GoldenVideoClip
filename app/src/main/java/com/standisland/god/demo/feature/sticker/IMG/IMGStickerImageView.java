package com.standisland.god.demo.feature.sticker.IMG;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.standisland.god.demo.common.utils.DisplayUtil;


/**
 * Created by felix on 2017/12/21 下午10:58.
 */

public class IMGStickerImageView extends IMGStickerView {

    private int MAX_SIZE = DisplayUtil.dip2px(this.getContext(), 100);

    private IMGBitmap mSticker;

    private ImageView mImageView;

    public IMGStickerImageView(Context context) {
        super(context);
    }

    public IMGStickerImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public IMGStickerImageView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    public View onCreateContentView(Context context, ViewGroup viewGroup) {
        mImageView = new ImageView(context);
        return mImageView;
    }

    public IMGBitmap getSticker() {
        return mSticker;
    }

    public void setSticker(IMGBitmap sticker) {
        if (sticker == null || mImageView == null) {
            return;
        }
        mSticker = sticker;
        String path = sticker.getFilePath();
        if (!TextUtils.isEmpty(path)) {
            Bitmap bitmap = BitmapFactory.decodeFile(path);
            resize(bitmap.getWidth(), bitmap.getHeight());
            mImageView.setImageBitmap(bitmap);
        }
        // TODO 贴字为空时
        mImageView.setVisibility(isVideoMode() ? INVISIBLE : VISIBLE);
    }


    private void resize(int width, int height) {
        if (width == 0 || height == 0) {
            return;
        }
        int size = Math.max(width, height);
        float f = 1.0f * MAX_SIZE / size;
        int h = (int) (height * f);
        int w = (int) (width * f);
        LayoutParams layoutParams = mImageView.getLayoutParams();
        layoutParams.width = w;
        layoutParams.height = h;
        mImageView.setLayoutParams(layoutParams);
    }


}
