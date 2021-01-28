package com.standisland.god.demo.feature.sticker;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import com.standisland.god.demo.common.utils.DisplayUtil;


public class CircleView extends View {
    private Paint paint;
    private int strokeWidth = 2;
    private int color = Color.WHITE;
    public CircleView(Context context) {
        this(context,null);
    }

    public CircleView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs,0);
    }

    public CircleView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }


    private void init() {
        paint = new Paint();
        paint.setColor(color);
        strokeWidth = DisplayUtil.dip2px(getContext(),strokeWidth);
        paint.setAntiAlias(true);
        paint.setStyle(Paint.Style.FILL);
    }

    public void setColor(int color) {
        this.color = color;
        postInvalidate();
    }

    public void setStrokeWidth(int width) {
        this.strokeWidth = DisplayUtil.dip2px(getContext(),width);
        postInvalidate();

    }

    public void setColorAndStrokeWidth(int color,int width){
        this.color = color;
        this.strokeWidth = DisplayUtil.dip2px(getContext(),width);
        postInvalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        float outX = getRight() - getLeft();
        float outY = getBottom() - getTop();
        paint.setColor(Color.WHITE);
        canvas.drawCircle(outX / 2f, outY / 2f, getWidth() / 2f, paint);
        paint.setColor(color);
        canvas.drawCircle(outX / 2f, outY / 2f, (getWidth()-strokeWidth) / 2f, paint);
    }
}
