package com.standisland.god.demo.feature.sticker;

import android.graphics.Bitmap;
import android.graphics.Matrix;

import com.standisland.god.demo.feature.sticker.IMG.IMGBitmap;
import com.standisland.god.demo.feature.sticker.IMG.IMGText;

import java.util.ArrayList;
import java.util.List;



/**
 * created by meiliqin
 * on 2020/7/9
 */
public class VideoStickerItem {

    private static final float DEFAULT_SCALE = 0.25f;

    private float left;
    private float top;
    private long startTimeMs;
    private long endTimeMs;
    private Bitmap bitmap;
    //private BitmapCache bitmapCache;
    private float scaleX=1,scaleY=1,rotate;
    private IMGText imgText;
    private boolean isText;
    private IMGBitmap imgBitmap;
    private String tag="";

    public VideoStickerItem(Bitmap bitmap, long startTimeMs, long endTimeMs, boolean isText) {
        this.isText = isText;
        float scale = isText ? DEFAULT_SCALE : 1.0f;
       // bitmapCache = new BitmapCache(scaleX, scaleY, rotate, bitmap, scale);
        this.bitmap=bitmap;
        this.startTimeMs = startTimeMs;
        this.endTimeMs = endTimeMs;
    }

    public void setImgText(IMGText imgText) {
        this.imgText = imgText;
    }


    public void setIsText(boolean text) {
        isText = text;
    }

    public void setImgBitmap(IMGBitmap imgBitmap) {
        this.imgBitmap = imgBitmap;
    }

    public IMGBitmap getImgBitmap() {
        return imgBitmap;
    }

    public IMGText getImgText() {
        return imgText;
    }

    public Bitmap getBitmap() {
//        if(rotate!=0){
//            Matrix matrix = new Matrix();
//            matrix.postRotate(rotate);
//            return Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
//        }
        //return bitmapCache.getBitmap(scaleX,scaleY,rotate);
        return bitmap;
    }

    public void setScaleX(float scaleX) {
        this.scaleX = scaleX;
    }

    public void setScaleY(float scaleY) {
        this.scaleY = scaleY;
    }

    public float getScaleX() {
        return scaleX;
    }

    public void setRotate(float rotate) {
        this.rotate = rotate;
    }

    public float getLeft() {
        return left;
    }

    public float getTop() {
        return top;
    }

    public void setLeft(float left) {
        this.left = left;
    }

    public void setTop(float top) {
        this.top = top;
    }

    public long getStartTimeMs() {
        return startTimeMs;
    }

    public long getEndTimeMs() {
        return endTimeMs;
    }

    public void setStartTimeMs(long startTimeMs) {
        this.startTimeMs = startTimeMs;
    }

    public void setEndTimeMs(long endTimeMs) {
        this.endTimeMs = endTimeMs;
    }

    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

//    public void updateBitmap(Bitmap bitmap) {
//        if (bitmapCache != null) {
//            bitmapCache.updateSrc(bitmap);
//        }
//    }
//
//    public void release(){
//        if (bitmapCache.src != null && !bitmapCache.src.isRecycled()) {
//            bitmapCache.src.recycle();
//            bitmapCache.src = null;
//        }
//        if (bitmapCache.ret != null && !bitmapCache.ret.isRecycled()) {
//            bitmapCache.ret.recycle();
//            bitmapCache.ret = null;
//        }
//    }


    private static class BitmapCache{
        float scaleX,scaleY,rotate;
        private Bitmap src,ret;
        private List<Bitmap> waste = new ArrayList<>();
        private float defaultScale;

        public BitmapCache(float scaleX, float scaleY, float rotate, Bitmap src, float defaultScale) {
            this.src = src;
            this.defaultScale = defaultScale;
            generateBitmap(scaleX,scaleY,rotate);
        }

        public void updateSrc(Bitmap bitmap){
            if(src!=null){
                waste.add(src);
            }
            src = bitmap;
            ret = null;
//            generateBitmap(scaleX,scaleY,rotate);
        }

        public void recycle(){
            for (int i = 0; i < waste.size(); i++) {
                waste.get(i).recycle();
            }
        }

        private Bitmap generateBitmap(float scaleX, float scaleY, float rotate){
            this.scaleX = scaleX;
            this.scaleY = scaleY;
            this.rotate = rotate;
            try {
                Matrix matrix = new Matrix();
                matrix.postScale(scaleX*defaultScale, scaleY*defaultScale);
                matrix.postRotate(rotate);
                int width = src.getWidth();
                int height = src.getHeight();
                ret = Bitmap.createBitmap(src, 0, 0, width, height, matrix, true);
            } catch (Exception e) {
                e.getMessage();
            }
            return ret;
        }

        public boolean needUpdateBitmap(float scaleX, float scaleY, float rotate){
            if(this.scaleY == scaleY && this.scaleX ==scaleX && this.rotate == rotate ){
                return false;
            }
            if(src!=ret) {
                ret.recycle();
            }
            return true;
        }

        public Bitmap getBitmap(float scaleX, float scaleY, float rotate){
           // recycle();
            if(ret==null){
                return generateBitmap(scaleX,scaleY,rotate);
            }
            if(needUpdateBitmap(scaleX,scaleY,rotate)){
                return generateBitmap(scaleX,scaleY,rotate);
            }
            return ret;
        }

    }

}