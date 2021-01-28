package com.standisland.god.demo.common.utils;

import android.content.ComponentCallbacks;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.util.DisplayMetrics;
import android.view.WindowManager;




public class DisplayUtil {

    private static float DEFAULT_UI_DPI = 375;
    private static float sNoncompatDensity;
    private static float sNoncompatScaledDensity;

    public static float mTargetDensity;
    public static float mTargetScaledDensity;
    public static int mTargetDpi;

    /*
       px = density * dp;
       density = dpi / 160;
       px = dp * (dpi / 160);
     */
    public static void setCustomDensity(Context context) {
        if (context == null) {
            return;
        }
        final DisplayMetrics appDisplayMetrics = context.getResources().getDisplayMetrics();
        if (sNoncompatDensity == 0) {
            sNoncompatDensity = appDisplayMetrics.density;
            sNoncompatScaledDensity = appDisplayMetrics.scaledDensity;
            context.registerComponentCallbacks(new ComponentCallbacks() {
                @Override
                public void onConfigurationChanged(Configuration newConfig) {
                    if (newConfig != null && newConfig.fontScale > 0) {
                        sNoncompatScaledDensity = context.getResources().getDisplayMetrics().scaledDensity;
                    }
                }

                @Override
                public void onLowMemory() {
                }
            });
        }
        mTargetDensity = appDisplayMetrics.widthPixels / DEFAULT_UI_DPI;
        mTargetScaledDensity = mTargetDensity * (sNoncompatScaledDensity / sNoncompatDensity);
        mTargetDpi = (int) (160 * mTargetDensity);


    }


    public static int dip2px(Context context,float dipValue) {
        final float scale = getDisplayDensity(context);
        return (int) (dipValue * scale + 0.5f);
    }

    public static int px2dip(Context context, float pxValue) {
        final float scale = getDisplayDensity(context);
        return (int) (pxValue / scale + 0.5f);
    }

    public static float sp2px(Context context, float spValue) {
        final float fontScale = getScaleDensity(context);
        return (spValue * fontScale + 0.5f);
    }

    public static float px2sp(Context context, float pxValue) {
        final float fontScale = getScaleDensity(context);
        return (pxValue / fontScale + 0.5f);
    }

    private static float getScaleDensity(Context context) {
        Resources resources = context.getApplicationContext().getResources();
        DisplayMetrics dm = resources.getDisplayMetrics();
        return dm.scaledDensity;
    }

    private static float getDisplayDensity(Context context) {
        Resources resources = context.getApplicationContext().getResources();
        DisplayMetrics dm = resources.getDisplayMetrics();
        return dm.density;
    }

    public static int getStatusBarHeight(Context context) {
        int result = 0;
        int resourceId = context.getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            result = context.getResources().getDimensionPixelSize(resourceId);
        }
        return result;
    }

    public static int getWindowWidth(Context context) {
        WindowManager manager = (WindowManager) context
                .getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics outMetrics = new DisplayMetrics();
        manager.getDefaultDisplay().getMetrics(outMetrics);
        return outMetrics.widthPixels;
    }

    public static int getWindowHeight(Context context) {
        WindowManager manager = (WindowManager) context
                .getSystemService(Context.WINDOW_SERVICE);
        DisplayMetrics outMetrics = new DisplayMetrics();
        manager.getDefaultDisplay().getMetrics(outMetrics);
        return outMetrics.heightPixels;
    }
}
