package com.standisland.god.demo.feature.sticker.IMG;

/**
 * author : linzheng
 * e-mail : linzheng@corp.standisland.com
 * time   : 2020/7/8
 * desc   :
 * version: 1.0
 */
public interface IMGStickerChangedListener {

    void onStickerRemoved(IMGStickerView stickerView);

    void onStickerTranslation(IMGStickerView stickerView, float translationX, float translationY);

    void onStickerAdd(IMGStickerView stickerView);

    void onStickerScale(IMGStickerView stickerView, float scale, float rotation);
//
//    void onEditBegin(IMGStickerView stickerView);
//
//    void onEditEnd(IMGStickerView stickerView);

    void onContentUpdate(IMGStickerView stickerView);

    // TODO 添加旋转，缩放回调
}
