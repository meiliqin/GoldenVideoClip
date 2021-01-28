package com.standisland.god.demo.feature.sticker;

import android.content.Context;
import android.content.res.TypedArray;
import android.text.TextUtils;
import android.util.AttributeSet;

import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatEditText;

import com.standisland.god.demo.R;
import com.standisland.god.demo.common.utils.FileUtil;
import com.standisland.god.demo.common.utils.FontUtils;


public class FontEditText extends AppCompatEditText {
    private String fontPath;
    public FontEditText(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.FontTextView, 0, 0);
        fontPath = typedArray.getString(R.styleable.FontTextView_customFont);
        if (!TextUtils.isEmpty(fontPath)) {
            FontUtils.getInstance().replaceFontFromFile(this, fontPath);
        }

        typedArray.recycle();
    }

    public void setFontPath(String fontPath) {
        if (!TextUtils.isEmpty(fontPath) && !fontPath.equals(this.fontPath) && FileUtil.isFileExists(fontPath)) {
            FontUtils.getInstance().replaceFontFromFile(this, fontPath);
        }
        this.fontPath = fontPath;
    }
}
