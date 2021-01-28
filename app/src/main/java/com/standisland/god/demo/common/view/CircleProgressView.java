package com.standisland.god.demo.common.view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.core.content.ContextCompat;

import com.standisland.god.demo.R;


/**
 * Created by linzheng on 2018/6/26.
 */

public class CircleProgressView extends View {

    // 整体(圆形)颜色
    private static final int DEFAULT_WHOLE_COLOR_ID = R.color.transparent;
    // 进度条(扇形)颜色
    private static final int DEFAULT_PROGRESS_COLOR_ID = R.color.circle_process_default_color;
    private static final int DEFAULT_OUTLINE_COLOR = Color.TRANSPARENT;
    private static final float DEFAULT_WIDTH = 12;

    private boolean clockwise;
    private Paint progressPaint;
    private int progressColorId;
    private Paint wholePaint;
    private int wholeColorId;
    private float progressRadius;
    private RectF progressRect;

    private Paint outlinePaint;
    private int outlineColor;
    private float outlineWidth;

    private float progress = 0f;


    public CircleProgressView(Context context) {
        this(context, null);
    }

    public CircleProgressView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CircleProgressView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initAttrs(context, attrs, defStyleAttr);
        initPaint();
    }

    private void initAttrs(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.CircleProgressView, defStyleAttr, 0);
        clockwise = ta.getBoolean(R.styleable.CircleProgressView_cpv_progress_direction, true);
        wholeColorId = ta.getResourceId(R.styleable.CircleProgressView_cpv_whole_color, -1);
        if (wholeColorId == -1) {
            wholeColorId = DEFAULT_WHOLE_COLOR_ID;
        }
        progressColorId = ta.getResourceId(R.styleable.CircleProgressView_cpv_progress_color, -1);
        if (progressColorId == -1){
            progressColorId = DEFAULT_PROGRESS_COLOR_ID;
        }

        progressRadius = ta.getDimension(R.styleable.CircleProgressView_cpv_progress_radius, DEFAULT_WIDTH);
        outlineColor = ta.getColor(R.styleable.CircleProgressView_cpv_outline_color, DEFAULT_OUTLINE_COLOR);
        outlineWidth = ta.getDimension(R.styleable.CircleProgressView_cpv_outline_width, DEFAULT_WIDTH);
        ta.recycle();
    }

    private void initPaint() {
        wholePaint = new Paint();
        wholePaint.setColor(ContextCompat.getColor(getContext(),wholeColorId));
        wholePaint.setAntiAlias(true);
        wholePaint.setStyle(Paint.Style.FILL);

        progressPaint = new Paint();
        progressPaint.setColor(ContextCompat.getColor(getContext(),progressColorId));
        progressPaint.setAntiAlias(true);
        progressPaint.setStyle(Paint.Style.FILL);
        progressRect = new RectF();

        outlinePaint = new Paint();
        outlinePaint.setColor(outlineColor);
        outlinePaint.setStrokeWidth(outlineWidth);
        outlinePaint.setAntiAlias(true);
        outlinePaint.setStyle(Paint.Style.STROKE);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int width = getWidth();
        int height = getHeight();
        if (width == 0 || height == 0) {
            measure(0, 0);
            return;
        }
        float radius = Math.min(width, height) / 2.0f - outlineWidth / 2.0f;
        // 画圆形
        if (wholeColorId != DEFAULT_WHOLE_COLOR_ID) {
            canvas.drawCircle(width / 2.0f, height / 2.0f, radius, wholePaint);
        }
        // 画边框
        canvas.drawCircle(width / 2.0f, height / 2.0f, radius, outlinePaint);
        // 画扇形
        progressRect.left = width / 2.0f - progressRadius;
        progressRect.right = width / 2.0f + progressRadius;
        progressRect.top = height / 2.0f - progressRadius;
        progressRect.bottom = height / 2.0f + progressRadius;
        float sweepAngle;
        if (clockwise) {
            sweepAngle = 360f * progress / 100;
        } else {
            sweepAngle = -360f * progress / 100;
        }
        canvas.drawArc(progressRect, -90f, sweepAngle, true, progressPaint);
    }

    public void setProgress(float progress) {
        this.progress = progress;
        invalidate();
    }


}
