package com.standisland.god.demo.feature.sticker.IMG;

import androidx.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;



/**
 * Created by felix on 2017/11/14 下午7:31.
 */

public interface IMGSticker extends IMGStickerPortrait, IMGViewPortrait, IMGMediaPortrait {

    int MODE_IMAGE = 0;
    int MODE_VIDEO = 1;

    @Retention(RetentionPolicy.SOURCE)
    @IntDef({MODE_IMAGE, MODE_VIDEO})
    @interface Mode {
    }


}
