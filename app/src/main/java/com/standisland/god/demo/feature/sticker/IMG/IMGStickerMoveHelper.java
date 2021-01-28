package com.standisland.god.demo.feature.sticker.IMG;

import android.graphics.Matrix;
import android.view.MotionEvent;
import android.view.View;

/**
 * Created by felix on 2017/11/17 下午6:08.
 */

public class IMGStickerMoveHelper {

    private static final String TAG = "IMGStickerMoveHelper";

    private IMGStickerView mView;

    private float mX, mY;

    private static final Matrix M = new Matrix();

    public IMGStickerMoveHelper(IMGStickerView view) {
        mView = view;
    }

    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mX = event.getX();
                mY = event.getY();
                M.reset();
                M.setRotate(v.getRotation());
                return true;
            case MotionEvent.ACTION_MOVE:
                float[] dxy = {event.getX() - mX, event.getY() - mY};
                M.mapPoints(dxy);

                float translationX = mView.getTranslationX() + dxy[0];
                float translationY = mView.getTranslationY() + dxy[1];
                v.setTranslationX(translationX);
                v.setTranslationY(translationY);
                mView.onStickerTranslation(translationX, translationY);
                return true;
        }
        return false;
    }
}
