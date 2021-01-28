package com.standisland.god.demo.feature.sticker.IMG;

import android.view.View;

/**
 * author : linzheng
 * e-mail : linzheng@corp.standisland.com
 * time   : 2020/7/6
 * desc   :
 * version: 1.0
 */
public class IMGStickerVisibleHelper {


    private IMGStickerView mStickerView;


    public IMGStickerVisibleHelper(IMGStickerView stickerView) {
        mStickerView = stickerView;
    }


    public void updateVisibility(long mediaPosition) {
        long start = mStickerView.getStartPosition();
        long end = mStickerView.getEndPosition();
        if (start == -1 && end == -1) {
            // TODO Dodge 未设置
            return;
        } else {
            boolean visible = mediaPosition >= start && mediaPosition <= end;
            // TODO Dodge 内容不可见，边框可见
            if (visible) {
                mStickerView.setVisibility(View.VISIBLE);
            } else {
                mStickerView.setVisibility(View.GONE);
            }
        }
    }


}
