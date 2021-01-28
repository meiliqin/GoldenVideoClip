package com.standisland.god.demo.common.utils;

import android.content.Context;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;

/**
 * Created by vincent on 9/8/15.
 *
 */
public class KeyboardUtil {

    public static void showKeyboard(final View view) {
        if (view == null) {
            return;
        }
        view.requestFocus();
        InputMethodManager inputManager = (InputMethodManager) view.getContext().getSystemService(
                        Context.INPUT_METHOD_SERVICE);
        inputManager.showSoftInput(view, 0);
    }

    /**
     * {@link #showKeyboard(View)}方法在dialog回来，有无法弹开键盘的情况发生
     *
     * 可以试试这个方法
     * */
    public static void showKeyboardEx(View view){
        InputMethodManager imm = (InputMethodManager)view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);

        if (!imm.isActive(view)){
            view.requestFocus();
            imm.toggleSoftInput(0, 0);
        }
    }

    public static void hideKeyboard(final View view) {
        if (view == null) {
            return;
        }

        InputMethodManager imm = (InputMethodManager) view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        view.clearFocus();
        imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }

    private static class KeyboardHeightWatcher implements ViewTreeObserver.OnGlobalLayoutListener {

        @Override
        public void onGlobalLayout() {

        }
    }

    public static boolean isActiveForInput(View view){
        InputMethodManager imm = (InputMethodManager)view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        return imm.isActive(view);
    }

}
