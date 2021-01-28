package com.standisland.god.demo.feature.sticker.IMG;

import android.text.TextUtils;

import java.util.Objects;

/**
 * Created by felix on 2017/12/1 下午2:43.
 */

public class IMGText {

    private String text;
    private int color;
    private boolean textBg;
    private String fontName;
    private String fontPath;

    public IMGText(String text, int color) {
        this.text = text;
        this.color = color;
    }

    /**
     * 添加属性时，记得维护这个set方法
     *
     * @param imgText text
     */
    public void set(IMGText imgText) {
        this.text = imgText.text;
        this.color = imgText.color;
        this.textBg = imgText.textBg;
        this.fontName = imgText.fontName;
        this.fontPath = imgText.fontPath;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    public int getColor() {
        return color;
    }

    public void setColor(int color) {
        this.color = color;
    }

    public boolean isTextBg() {
        return textBg;
    }

    public void setTextBg(boolean textBg) {
        this.textBg = textBg;
    }

    public void setFontName(String fontName) {
        this.fontName = fontName;
    }

    public String getFontName() {
        return fontName;
    }

    public String getFontPath() {
        return fontPath;
    }

    public void setFontPath(String fontPath) {
        this.fontPath = fontPath;
    }

    public boolean isEmpty() {
        return TextUtils.isEmpty(text);
    }

    public int length() {
        return isEmpty() ? 0 : text.length();
    }

    @Override
    public String toString() {
        return "IMGText{" +
                "text='" + text + '\'' +
                ", color=" + color +
                ", textBg=" + textBg +
                ", fontName='" + fontName + '\'' +
                ", fontPath='" + fontPath + '\'' +
                '}';
    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        IMGText imgText = (IMGText) o;
        return color == imgText.color &&
                textBg == imgText.textBg &&
                Objects.equals(text, imgText.text) &&
                Objects.equals(fontName, imgText.fontName) &&
                Objects.equals(fontPath, imgText.fontPath);
    }

    @Override
    public int hashCode() {
        return Objects.hash(text, color, textBg, fontName, fontPath);
    }

}
