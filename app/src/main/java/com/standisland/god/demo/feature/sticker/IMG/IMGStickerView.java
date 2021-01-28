package com.standisland.god.demo.feature.sticker.IMG;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.ImageView;

import com.standisland.god.demo.R;
import com.standisland.god.demo.common.utils.DisplayUtil;


/**
 * Created by felix on 2017/12/12 下午4:26.
 */

public abstract class IMGStickerView extends ViewGroup implements IMGSticker, View.OnClickListener {

    private static final String TAG = "IMGStickerView";

    private IMGMediaPosition mMediaPosition;

    private View mContentView;

    private float mScale = 1f;

    @Mode
    protected int mStickerMode = MODE_IMAGE;

    // TODO
    private int mDownShowing = 0;

    private IMGStickerMoveHelper mMoveHelper;

    protected IMGStickerHelper<IMGStickerView> mStickerHelper;

    private IMGStickerVisibleHelper mVisibleHelper;

    private ImageView mRemoveView, mAdjustView;

    private ImageView mDurationView;

    private float mMaxScaleValue = MAX_SCALE_VALUE;

    private Paint PAINT;

    private Matrix mMatrix = new Matrix();

    private RectF mFrame = new RectF();

    private Rect mTempFrame = new Rect();

    private IMGStickerChangedListener mParentStickerListener;

    private static final float MAX_SCALE_VALUE = 4f;

    private static final int ANCHOR_SIZE = 35*3;

    private static final int MARGIN_SIZE = ANCHOR_SIZE >> 1;

    private static final int ANCHOR_SIZE_HALF = ANCHOR_SIZE >> 1;

    private static final float STROKE_WIDTH = 3f;

    {
        PAINT = new Paint(Paint.ANTI_ALIAS_FLAG);
        PAINT.setColor(Color.WHITE);
        PAINT.setStyle(Paint.Style.STROKE);
        PAINT.setStrokeWidth(STROKE_WIDTH);
    }

    public IMGStickerView(Context context) {
        this(context, null, 0);
    }

    public IMGStickerView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public IMGStickerView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        onInitialize(context);
    }

    public void onInitialize(Context context) {
        setBackgroundColor(Color.TRANSPARENT);

        mContentView = onCreateContentView(context, this);
        addView(mContentView, getContentLayoutParams());

        mRemoveView = new ImageView(context);
        mRemoveView.setScaleType(ImageView.ScaleType.FIT_XY);
        mRemoveView.setImageResource(R.drawable.ic_sticker_delete);
        addView(mRemoveView, getAnchorLayoutParams());
        mRemoveView.setOnClickListener(this);

        mAdjustView = new ImageView(context);
        mAdjustView.setScaleType(ImageView.ScaleType.FIT_XY);
        mAdjustView.setImageResource(R.drawable.ic_sticker_roate);
        addView(mAdjustView, getAnchorLayoutParams());

        new IMGStickerAdjustHelper(this, mAdjustView);

        mStickerHelper = new IMGStickerHelper<>(this);
        mMoveHelper = new IMGStickerMoveHelper(this);
        mVisibleHelper = new IMGStickerVisibleHelper(this);
    }

    public abstract View onCreateContentView(Context context, ViewGroup viewGroup);

    public void setMediaPosition(IMGMediaPosition mediaPosition) {
        mMediaPosition = mediaPosition;
    }

    public boolean isVideoMode() {
        return mStickerMode == MODE_VIDEO;
    }

    @Mode
    public int getStickerMode() {
        return mStickerMode;
    }

    public void setStickerMode(@Mode int stickerMode) {
        mStickerMode = stickerMode;
        if (stickerMode == MODE_VIDEO) {
            addDurationView();
        }
    }

    private void addDurationView() {
        if (mDurationView == null) {
            mDurationView = new ImageView(getContext());
            mDurationView.setScaleType(ImageView.ScaleType.FIT_XY);
            mDurationView.setImageResource(R.drawable.ic_sticker_duration);
            addView(mDurationView, getAnchorLayoutParams());
            mDurationView.setOnClickListener(v -> mStickerHelper.onDurationClick(this));
        }
    }

    @Override
    public float getScale() {
        return mScale;
    }

    @Override
    public void setScale(float scale) {
        mScale = scale;

        mContentView.setScaleX(mScale);
        mContentView.setScaleY(mScale);

        int pivotX = (getLeft() + getRight()) >> 1;
        int pivotY = (getTop() + getBottom()) >> 1;

        mFrame.set(pivotX, pivotY, pivotX, pivotY);
        mFrame.inset(-(mContentView.getMeasuredWidth() >> 1), -(mContentView.getMeasuredHeight() >> 1));

        mMatrix.setScale(mScale, mScale, mFrame.centerX(), mFrame.centerY());
        mMatrix.mapRect(mFrame);

        mFrame.inset(-MARGIN_SIZE, -MARGIN_SIZE);
        mFrame.round(mTempFrame);

        // 保存缩放后的测量结果
        setMeasuredDimension(mTempFrame.width(), mTempFrame.height());

        layout(mTempFrame.left, mTempFrame.top, mTempFrame.right, mTempFrame.bottom);
        // 回调旋转，缩放
        IMGStickerChangedListener listener = getStickerChangedListener();
        if (listener != null) {
            listener.onStickerScale(this, scale, getRotation());
        }
    }

    @Override
    public void addScale(float scale) {
        setScale(getScale() * scale);
    }

    private LayoutParams getContentLayoutParams() {
        return new MarginLayoutParams(
                LayoutParams.WRAP_CONTENT,
                LayoutParams.WRAP_CONTENT
        );
    }

    private LayoutParams getAnchorLayoutParams() {
        return new MarginLayoutParams(ANCHOR_SIZE, ANCHOR_SIZE);
    }

    @Override
    public void draw(Canvas canvas) {
        if (isShowing()) {
            canvas.drawRect(ANCHOR_SIZE_HALF, ANCHOR_SIZE_HALF,
                    getWidth() - ANCHOR_SIZE_HALF,
                    getHeight() - ANCHOR_SIZE_HALF, PAINT);
        }
        super.draw(canvas);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int count = getChildCount();

        int maxHeight = 0;
        int maxWidth = 0;
        int childState = 0;

        for (int i = 0; i < count; i++) {
            final View child = getChildAt(i);
            if (child.getVisibility() != GONE) {
                measureChildWithMargins(child, widthMeasureSpec, 0, heightMeasureSpec, 0);
                float childWidth = child.getMeasuredWidth() * child.getScaleX() + MARGIN_SIZE * 2;
                float childHeight = child.getMeasuredHeight() * child.getScaleX() + MARGIN_SIZE * 2;
                maxWidth = Math.round(Math.max(maxWidth, childWidth));
                maxHeight = Math.round(Math.max(maxHeight, childHeight));
                childState = combineMeasuredStates(childState, child.getMeasuredState());
            }
        }

        maxHeight = Math.max(maxHeight, getSuggestedMinimumHeight());
        maxWidth = Math.max(maxWidth, getSuggestedMinimumWidth());

//        int width = Math.max(maxWidth, resolveSizeAndState(maxWidth, widthMeasureSpec, childState));
//        int height = Math.max(maxHeight, resolveSizeAndState(maxHeight, heightMeasureSpec, childState << MEASURED_HEIGHT_STATE_SHIFT));

        setMeasuredDimension(maxWidth, maxHeight);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {

        mFrame.set(left, top, right, bottom);

        int count = getChildCount();
        if (count == 0) {
            return;
        }

        mRemoveView.layout(0, 0, mRemoveView.getMeasuredWidth(), mRemoveView.getMeasuredHeight());
        mAdjustView.layout(
                right - left - mAdjustView.getMeasuredWidth(),
                bottom - top - mAdjustView.getMeasuredHeight(),
                right - left, bottom - top
        );

        if (isVideoMode() && mDurationView != null) {
            mDurationView.layout(
                    right - left - mDurationView.getMeasuredWidth(),
                    0,
                    right - left,
                    mDurationView.getMeasuredHeight()
            );
        }

        int centerX = (right - left) >> 1, centerY = (bottom - top) >> 1;
        int hw = mContentView.getMeasuredWidth() >> 1;
        int hh = mContentView.getMeasuredHeight() >> 1;

        mContentView.layout(centerX - hw, centerY - hh, centerX + hw, centerY + hh);
    }

    @Override
    protected boolean drawChild(Canvas canvas, View child, long drawingTime) {
        return (isShowing() || isVideoMode()) && super.drawChild(canvas, child, drawingTime);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent ev) {
        if (ev.getActionMasked() == MotionEvent.ACTION_DOWN) {
            requestDisallowInterceptTouchEvent(true);
        }
        if (!isShowing() && ev.getAction() == MotionEvent.ACTION_DOWN) {
            mDownShowing = 0;
            show();
            return true;
        }
        return isShowing() && super.onInterceptTouchEvent(ev);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

        boolean handled = mMoveHelper.onTouch(this, event);

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                mDownShowing++;
                requestDisallowInterceptTouchEvent(true);
                break;
            case MotionEvent.ACTION_UP:
                if (mDownShowing > 1 && event.getEventTime() - event.getDownTime() < ViewConfiguration.getTapTimeout()) {
                    onContentTap();
                    return true;
                }
                break;
        }

        return handled | super.onTouchEvent(event);
    }

    @Override
    public void onClick(View v) {
        if (v == mRemoveView) {
            onRemove();
        }
    }

    public void onRemove() {
        mStickerHelper.remove();
    }

    public void onContentTap() {

    }

    @Override
    public boolean show() {
//        if(getStickerChangedListener()!=null){
//            mParentStickerListener.onEditBegin(this);
//        }
        setFrameVisible(true);
        return mStickerHelper.show();
    }

    @Override
    public boolean remove() {
        return mStickerHelper.remove();
    }

    @Override
    public boolean dismiss() {
//        if(getStickerChangedListener()!=null){
//            mParentStickerListener.onEditEnd(this);
//        }
        setFrameVisible(false);
        return mStickerHelper.dismiss();
    }

    @Override
    public boolean isShowing() {
        return mStickerHelper.isShowing();
    }

    @Override
    public RectF getFrame() {
        return mStickerHelper.getFrame();
    }

    @Override
    public void onSticker(Canvas canvas) {
        canvas.translate(mContentView.getX(), mContentView.getY());
        mContentView.draw(canvas);
    }

    @Override
    public void registerCallback(Callback callback) {
        mStickerHelper.registerCallback(callback);
    }

    @Override
    public void unregisterCallback(Callback callback) {
        mStickerHelper.unregisterCallback(callback);
    }

    @Override
    public long getStartPosition() {
        return mMediaPosition != null ? mMediaPosition.getStart() : -1;
    }

    @Override
    public long getEndPosition() {
        return mMediaPosition != null ? mMediaPosition.getEnd() : -1;
    }

    public void updateVisibility(long mediaPosition) {
        if (mMediaPosition != null) {
            mVisibleHelper.updateVisibility(mediaPosition);
        }
    }

    public IMGStickerChangedListener getStickerChangedListener() {
        if (mParentStickerListener == null) {
            ViewParent viewParent = getParent();
            if (viewParent instanceof IMGView) {
                mParentStickerListener = ((IMGView) viewParent).getStickerChangedListener();
            }
        }
        return mParentStickerListener;
    }

    public void onStickerTranslation(float translationX, float translationY) {
        IMGStickerChangedListener listener = getStickerChangedListener();
        if (listener != null) {
            listener.onStickerTranslation(this, translationX, translationY);
        }
    }

    public View getContentView() {
        return mContentView;
    }


    private void setFrameVisible(boolean visible) {
        mAdjustView.setVisibility(visible ? VISIBLE : GONE);
        mRemoveView.setVisibility(visible ? VISIBLE : GONE);
        if (mDurationView != null) {
            mDurationView.setVisibility(visible ? VISIBLE : GONE);
        }
    }

    public Bitmap buildContentBitmap(View contentView) {
        int width = contentView.getWidth();
        int height = contentView.getHeight();
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        contentView.draw(canvas);
        return bitmap;
    }

    public Bitmap buildContentBitmap(View contentView, int scale) {
        int width = contentView.getWidth();
        int height = contentView.getHeight();
        Bitmap bitmap = Bitmap.createBitmap(width * scale, height * scale, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        canvas.scale(scale, scale);
        contentView.draw(canvas);
        return bitmap;
    }

}
