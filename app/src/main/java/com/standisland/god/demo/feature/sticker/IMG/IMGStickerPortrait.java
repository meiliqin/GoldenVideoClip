package com.standisland.god.demo.feature.sticker.IMG;

import android.graphics.Canvas;
import android.graphics.RectF;
import android.view.View;


/**
 * Created by felix on 2017/11/16 下午5:54.
 */

public interface IMGStickerPortrait {

    boolean show();

    boolean remove();

    boolean dismiss();

    boolean isShowing();

    RectF getFrame();

//    RectF getAdjustFrame();
//
//    RectF getDeleteFrame();

    void onSticker(Canvas canvas);

    void registerCallback(IMGSticker.Callback callback);

    void unregisterCallback(IMGSticker.Callback callback);

    interface Callback {

        <V extends View & IMGSticker> void onDismiss(V stickerView);

        <V extends View & IMGSticker> void onShowing(V stickerView);

        <V extends View & IMGSticker> boolean onRemove(V stickerView);

        void onDurationClick(IMGStickerView stickerView);

        void onContentUpdate(IMGStickerView stickerView);
    }
}
