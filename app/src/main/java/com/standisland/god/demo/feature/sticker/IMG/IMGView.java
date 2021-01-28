package com.standisland.god.demo.feature.sticker.IMG;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.CornerPathEffect;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.ViewParent;
import android.widget.FrameLayout;

import androidx.annotation.Nullable;


/**
 * Created by felix on 2017/11/14 下午6:43.
 */
// TODO clip外不加入path
public class IMGView extends FrameLayout implements Runnable, ScaleGestureDetector.OnScaleGestureListener,
        ValueAnimator.AnimatorUpdateListener, IMGStickerPortrait.Callback, Animator.AnimatorListener {

    private static final String TAG = "IMGView";

    private boolean mGestureEnable = false;

    private IMGMode mPreMode = IMGMode.NONE;

    private IMGImage mImage = new IMGImage();

    private GestureDetector mGDetector;

    private ScaleGestureDetector mSGDetector;

    private IMGHomingAnimator mHomingAnimator;

    private Pen mPen = new Pen();

    private int mPointerCount = 0;

    private Paint mDoodlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);

    private Paint mMosaicPaint = new Paint(Paint.ANTI_ALIAS_FLAG);

    private IMGStickerChangedListener mStickerChangedListener;

    private IMGStickerDurationClickListener mDurationClickListener;

    private static final boolean DEBUG = true;

    {
        // 涂鸦画刷
        mDoodlePaint.setStyle(Paint.Style.STROKE);
        mDoodlePaint.setStrokeWidth(IMGPath.BASE_DOODLE_WIDTH);
        mDoodlePaint.setColor(Color.RED);
        mDoodlePaint.setPathEffect(new CornerPathEffect(IMGPath.BASE_DOODLE_WIDTH));
        mDoodlePaint.setStrokeCap(Paint.Cap.ROUND);
        mDoodlePaint.setStrokeJoin(Paint.Join.ROUND);

        // 马赛克画刷
        mMosaicPaint.setStyle(Paint.Style.STROKE);
        mMosaicPaint.setStrokeWidth(IMGPath.BASE_MOSAIC_WIDTH);
        mMosaicPaint.setColor(Color.BLACK);
        mMosaicPaint.setPathEffect(new CornerPathEffect(IMGPath.BASE_MOSAIC_WIDTH));
        mMosaicPaint.setStrokeCap(Paint.Cap.ROUND);
        mMosaicPaint.setStrokeJoin(Paint.Join.ROUND);
    }

    public IMGView(Context context) {
        this(context, null, 0);
    }

    public IMGView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public IMGView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initialize(context);
    }

    private void initialize(Context context) {
        mPen.setMode(mImage.getMode());
        mGDetector = new GestureDetector(context, new MoveAdapter());
        mSGDetector = new ScaleGestureDetector(context, this);
    }

    public IMGStickerChangedListener getStickerChangedListener() {
        return mStickerChangedListener;
    }

    public void setStickerChangedListener(IMGStickerChangedListener stickerChangedListener) {
        mStickerChangedListener = stickerChangedListener;
    }

    public IMGStickerDurationClickListener getDurationClickListener() {
        return mDurationClickListener;
    }

    public void setDurationClickListener(IMGStickerDurationClickListener durationClickListener) {
        mDurationClickListener = durationClickListener;
    }

    public void setGestureEnable(boolean gestureEnable) {
        mGestureEnable = gestureEnable;
    }

    public void setImageBitmap(Bitmap image) {
        mImage.setBitmap(image);
        invalidate();
    }

    public void setMode(IMGMode mode) {
        // 保存现在的编辑模式
        mPreMode = mImage.getMode();

        // 设置新的编辑模式
        mImage.setMode(mode);
        mPen.setMode(mode);

        // 矫正区域
        onHoming();
    }

    public void updateMediaPosition(long mediaPosition) {
        mImage.updateMediaPosition(mediaPosition);
    }

    /**
     * 是否真正修正归位
     */
    boolean isHoming() {
        return mHomingAnimator != null
                && mHomingAnimator.isRunning();
    }

    private void onHoming() {
        invalidate();
        stopHoming();
        startHoming(mImage.getStartHoming(getScrollX(), getScrollY()),
                mImage.getEndHoming(getScrollX(), getScrollY()));
    }

    private void startHoming(IMGHoming sHoming, IMGHoming eHoming) {
        if (mHomingAnimator == null) {
            mHomingAnimator = new IMGHomingAnimator();
            mHomingAnimator.addUpdateListener(this);
            mHomingAnimator.addListener(this);
        }
        mHomingAnimator.setHomingValues(sHoming, eHoming);
        mHomingAnimator.start();
    }

    private void stopHoming() {
        if (mHomingAnimator != null) {
            mHomingAnimator.cancel();
        }
    }

    public void doRotate() {
        if (!isHoming()) {
            mImage.rotate(-90);
            onHoming();
        }
    }

    public void doRotate(boolean clockwise) {
        if (!isHoming()) {
            mImage.rotate(clockwise ? 90 : -90);
            onHoming();
        }
    }

    public void resetClip() {
        mImage.resetClip();
        onHoming();
    }

    public void doClip() {
        mImage.clip(getScrollX(), getScrollY());
        setMode(mPreMode);
        onHoming();
    }

    public void cancelClip() {
        mImage.toBackupClip();
        setMode(mPreMode);
    }

    public void setPenColor(int color) {
        mPen.setColor(color);
    }

    public void setPenWidth(float width) {
        mPen.setWidth(width);
    }

    public boolean isDoodleEmpty() {
        return mImage.isDoodleEmpty();
    }

    public void undoDoodle() {
        mImage.undoDoodle();
        invalidate();
    }

    public boolean isMosaicEmpty() {
        return mImage.isMosaicEmpty();
    }

    public int getMosaicSize() {
        return mImage.getMosaicSize();
    }

    public void undoMosaic() {
        mImage.undoMosaic();
        invalidate();
    }

    public IMGMode getMode() {
        return mImage.getMode();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        onDrawImages(canvas, null);
    }

    /**
     * 绘制图层
     *
     * @param canvas canvas
     * @param bitmap 传入外部bitmap （IMGView 中的 bitmap 会在 onDetachedFromWindow() 回调是回收，这时可以通过传入 bitmap 来实现绘制）
     */
    private void onDrawImages(Canvas canvas, @Nullable Bitmap bitmap) {
        canvas.save();

        // clip 中心旋转
        RectF clipFrame = mImage.getClipFrame();
        canvas.rotate(mImage.getRotate(), clipFrame.centerX(), clipFrame.centerY());

        // 图片
        mImage.onDrawImage(canvas, bitmap);

        // 马赛克
        if (!mImage.isMosaicEmpty() || (mImage.getMode() == IMGMode.MOSAIC && !mPen.isEmpty())) {
            int count = mImage.onDrawMosaicsPath(canvas);
            if (mImage.getMode() == IMGMode.MOSAIC && !mPen.isEmpty()) {
                mDoodlePaint.setStrokeWidth(mPen.getWidth());
                canvas.save();
                RectF frame = mImage.getClipFrame();
                canvas.rotate(-mImage.getRotate(), frame.centerX(), frame.centerY());
                canvas.translate(getScrollX(), getScrollY());
                canvas.drawPath(mPen.getPath(), mDoodlePaint);
                canvas.restore();
            }
            mImage.onDrawMosaic(canvas, count, bitmap);
        }

        // 涂鸦
        mImage.onDrawDoodles(canvas);
        if (mImage.getMode() == IMGMode.DOODLE && !mPen.isEmpty()) {
            mDoodlePaint.setColor(mPen.getColor());
            mDoodlePaint.setStrokeWidth(IMGPath.BASE_DOODLE_WIDTH * mImage.getScale());
            canvas.save();
            RectF frame = mImage.getClipFrame();
            canvas.rotate(-mImage.getRotate(), frame.centerX(), frame.centerY());
            canvas.translate(getScrollX(), getScrollY());
            canvas.drawPath(mPen.getPath(), mDoodlePaint);
            canvas.restore();
        }

        // TODO
        if (mImage.isFreezing()) {
            // 文字贴片
            mImage.onDrawStickers(canvas);
        }

        mImage.onDrawShade(canvas);

        canvas.restore();

        // TODO
        if (!mImage.isFreezing()) {
            // 文字贴片
            // 不剪切 贴纸图层
//            mImage.onDrawStickerClip(canvas);
            mImage.onDrawStickers(canvas);
        }

        // 裁剪
        if (mImage.getMode() == IMGMode.CLIP) {
            canvas.save();
            canvas.translate(getScrollX(), getScrollY());
            mImage.onDrawClip(canvas, getScrollX(), getScrollY());
            canvas.restore();
        }
    }

    public void stickAll() {
        mImage.stickAll();
    }

    public Bitmap saveBitmap() {
        return saveBitmap(null);
    }

    public Bitmap saveBitmap(Bitmap srcBitmap) {
        post(() -> mImage.stickAll());

        float scale = 1f / mImage.getScale();

        RectF frame = new RectF(mImage.getClipFrame());

        // 旋转基画布
        Matrix m = new Matrix();
        m.setRotate(mImage.getRotate(), frame.centerX(), frame.centerY());
        m.mapRect(frame);

        // 缩放基画布
        m.setScale(scale, scale, frame.left, frame.top);
        m.mapRect(frame);

        Bitmap bitmap = Bitmap.createBitmap(Math.round(frame.width()),
                Math.round(frame.height()), Bitmap.Config.ARGB_8888);

        Canvas canvas = new Canvas(bitmap);

        // 平移到基画布原点&缩放到原尺寸
        canvas.translate(-frame.left, -frame.top);
        canvas.scale(scale, scale, frame.left, frame.top);

        onDrawImages(canvas, srcBitmap);

        return bitmap;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        if (changed) {
            mImage.onWindowChanged(right - left, bottom - top);
        }
    }

    public void addStickerView(IMGStickerView stickerView, LayoutParams params) {
        if (stickerView != null) {

            addView(stickerView, params);

            stickerView.registerCallback(this);
            mImage.addSticker(stickerView);

            if (mStickerChangedListener != null) {
                mStickerChangedListener.onStickerAdd(stickerView);
            }
        }
    }

    public void addStickerImage(@IMGSticker.Mode int mode, IMGBitmap imgBitmap) {
        addStickerImage(mode, imgBitmap, 0);
    }

    public void addStickerImage(@IMGSticker.Mode int mode, IMGBitmap imgBitmap, int offset) {
        IMGStickerImageView stickerView = new IMGStickerImageView(getContext());
        stickerView.setStickerMode(mode);
        stickerView.setSticker(imgBitmap);
        LayoutParams layoutParams = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);

        // Center of the drawing window.
        layoutParams.gravity = Gravity.CENTER;

        stickerView.setX(getScrollX() + offset);
        stickerView.setY(getScrollY() + offset);

        addStickerView(stickerView, layoutParams);
    }

    public void addStickerText(IMGText text) {
        addStickerText(IMGSticker.MODE_IMAGE, text);
    }

    public void addStickerText(@IMGSticker.Mode int mode, IMGText text) {
        addStickerText(mode, text, null);
    }

    public void addStickerText(@IMGSticker.Mode int mode, IMGText text, IMGStickerTextView.TextClickListener textClickListener) {
        addStickerText(mode, text, 0, textClickListener);
    }

    /**
     * 添加文字贴纸
     *
     * @param mode              贴纸模式
     * @param text              文字贴纸
     * @param offset            偏移量
     * @param textClickListener 贴纸点击监听
     */
    public void addStickerText(@IMGSticker.Mode int mode, IMGText text, int offset, IMGStickerTextView.TextClickListener textClickListener) {
        IMGStickerTextView stickerView = new IMGStickerTextView(getContext());
        stickerView.setStickerMode(mode);
        if (textClickListener != null) {
            stickerView.setTextClickListener(textClickListener);
        }
        stickerView.setText(text);

        LayoutParams layoutParams = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);

        // Center of the drawing window.
        layoutParams.gravity = Gravity.CENTER;

        stickerView.setX(getScrollX() + offset);
        stickerView.setY(getScrollY() + offset);

        addStickerView(stickerView, layoutParams);
    }


    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        if (ev.getActionMasked() == MotionEvent.ACTION_DOWN) {
            return onInterceptTouch(ev) || super.onInterceptTouchEvent(ev);
        }
        return super.onInterceptTouchEvent(ev);
    }

    boolean onInterceptTouch(MotionEvent event) {
        if (isHoming()) {
            stopHoming();
            return true;
        } else if (mImage.getMode() == IMGMode.CLIP) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                removeCallbacks(this);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                postDelayed(this, 1200);
                break;
        }
        return onTouch(event);
    }

    boolean onTouch(MotionEvent event) {

        if (isHoming()) {
            // Homing
            return false;
        }

        mPointerCount = event.getPointerCount();

        boolean handled = mSGDetector.onTouchEvent(event);

        IMGMode mode = mImage.getMode();

        if (mode == IMGMode.NONE || mode == IMGMode.CLIP) {
            handled |= onTouchNONE(event);
        } else if (mPointerCount > 1) {
            onPathDone();
            handled |= onTouchNONE(event);
        } else {
            handled |= onTouchPath(event);
        }

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mImage.onTouchDown(event.getX(), event.getY());
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                mImage.onTouchUp(getScrollX(), getScrollY());
                onHoming();
                break;
        }

        return handled;
    }


    private boolean onTouchNONE(MotionEvent event) {
        return mGDetector.onTouchEvent(event);
    }

    private boolean onTouchPath(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                return onPathBegin(event);
            case MotionEvent.ACTION_MOVE:
                return onPathMove(event);
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                return mPen.isIdentity(event.getPointerId(0)) && onPathDone();
        }
        return false;
    }

    private boolean onPathBegin(MotionEvent event) {
        mPen.reset(event.getX(), event.getY());
        mPen.setIdentity(event.getPointerId(0));
        return true;
    }

    private boolean onPathMove(MotionEvent event) {
        if (mPen.isIdentity(event.getPointerId(0))) {
            mPen.lineTo(event.getX(), event.getY());
            invalidate();
            return true;
        }
        return false;
    }

    private boolean onPathDone() {
        if (mPen.isEmpty()) {
            return false;
        }
        mImage.addPath(mPen.toPath(), getScrollX(), getScrollY());
        mPen.reset();
        invalidate();
        return true;
    }

    @Override
    public void run() {
        // 稳定触发
        if (!onSteady()) {
            postDelayed(this, 500);
        }
    }

    boolean onSteady() {
        if (DEBUG) {
            Log.d(TAG, "onSteady: isHoming=" + isHoming());
        }
        if (!isHoming()) {
            mImage.onSteady(getScrollX(), getScrollY());
            onHoming();
            return true;
        }
        return false;
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        removeCallbacks(this);
        mImage.release();
    }

    @Override
    public boolean onScale(ScaleGestureDetector detector) {
        if (!mGestureEnable) {
            return false;
        }

        if (mPointerCount > 1) {
            mImage.onScale(detector.getScaleFactor(),
                    getScrollX() + detector.getFocusX(),
                    getScrollY() + detector.getFocusY());
            invalidate();
            return true;
        }
        return false;
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector detector) {
        if (!mGestureEnable) {
            return false;
        }

        if (mPointerCount > 1) {
            mImage.onScaleBegin();
            return true;
        }
        return false;
    }

    @Override
    public void onScaleEnd(ScaleGestureDetector detector) {
        if (mGestureEnable) {
            mImage.onScaleEnd();
        }
    }

    @Override
    public void onAnimationUpdate(ValueAnimator animation) {
        mImage.onHoming(animation.getAnimatedFraction());
        toApplyHoming((IMGHoming) animation.getAnimatedValue());
    }


    private void toApplyHoming(IMGHoming homing) {
        mImage.setScale(homing.scale);
        mImage.setRotate(homing.rotate);
        if (!onScrollTo(Math.round(homing.x), Math.round(homing.y))) {
            invalidate();
        }
    }

    private boolean onScrollTo(int x, int y) {
        if (getScrollX() != x || getScrollY() != y) {
            scrollTo(x, y);
            return true;
        }
        return false;
    }

    @Override
    public <V extends View & IMGSticker> void onDismiss(V stickerView) {
        mImage.onDismiss(stickerView);
        invalidate();
    }

    @Override
    public <V extends View & IMGSticker> void onShowing(V stickerView) {
        mImage.onShowing(stickerView);
        invalidate();
    }

    @Override
    public <V extends View & IMGSticker> boolean onRemove(V stickerView) {
        if (mImage != null) {
            mImage.onRemoveSticker(stickerView);
        }
        stickerView.unregisterCallback(this);
        ViewParent parent = stickerView.getParent();
        if (parent != null) {
//            ((ViewGroup) parent).removeView(stickerView);
            stickerView.setVisibility(GONE);

        }
        if (mStickerChangedListener != null && stickerView instanceof IMGStickerView) {
            mStickerChangedListener.onStickerRemoved((IMGStickerView) stickerView);
        }
        return true;
    }

    @Override
    public void onDurationClick(IMGStickerView stickerView) {
        if (mDurationClickListener != null) {
            mDurationClickListener.onDurationClick(stickerView);
        }
    }

    @Override
    public void onContentUpdate(IMGStickerView stickerView) {
        if (mStickerChangedListener != null) {
            mStickerChangedListener.onContentUpdate(stickerView);
        }
    }

    @Override
    public void onAnimationStart(Animator animation) {
        if (DEBUG) {
            Log.d(TAG, "onAnimationStart");
        }
        mImage.onHomingStart(mHomingAnimator.isRotate());
    }

    @Override
    public void onAnimationEnd(Animator animation) {
        if (DEBUG) {
            Log.d(TAG, "onAnimationEnd");
        }
        if (mImage.onHomingEnd(getScrollX(), getScrollY(), mHomingAnimator.isRotate())) {
            toApplyHoming(mImage.clip(getScrollX(), getScrollY()));
        }
    }

    @Override
    public void onAnimationCancel(Animator animation) {
        if (DEBUG) {
            Log.d(TAG, "onAnimationCancel");
        }
        mImage.onHomingCancel(mHomingAnimator.isRotate());
    }

    @Override
    public void onAnimationRepeat(Animator animation) {
        // empty implementation.
    }

    private boolean onScroll(float dx, float dy) {
        IMGHoming homing = mImage.onScroll(getScrollX(), getScrollY(), -dx, -dy);
        if (homing != null) {
            toApplyHoming(homing);
            return true;
        }
        return onScrollTo(getScrollX() + Math.round(dx), getScrollY() + Math.round(dy));
    }

    public boolean isModify() {
        return mImage.isModify();
    }

    public boolean isClip() {
        return mImage.isClip();
    }

    public IMGImage getImage() {
        return mImage;
    }

    private class MoveAdapter extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onDown(MotionEvent e) {
            return mGestureEnable && true;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            return mGestureEnable && IMGView.this.onScroll(distanceX, distanceY);
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            return mGestureEnable && super.onFling(e1, e2, velocityX, velocityY);
        }
    }

    private static class Pen extends IMGPath {

        private int identity = Integer.MIN_VALUE;

        void reset() {
            this.path.reset();
            this.identity = Integer.MIN_VALUE;
        }

        void reset(float x, float y) {
            this.path.reset();
            this.path.moveTo(x, y);
            this.identity = Integer.MIN_VALUE;
        }

        void setIdentity(int identity) {
            this.identity = identity;
        }

        boolean isIdentity(int identity) {
            return this.identity == identity;
        }

        void lineTo(float x, float y) {
            this.path.lineTo(x, y);
        }

        boolean isEmpty() {
            return this.path.isEmpty();
        }

        IMGPath toPath() {
            return new IMGPath(new Path(this.path), getMode(), getColor(), getWidth());
        }
    }
}
