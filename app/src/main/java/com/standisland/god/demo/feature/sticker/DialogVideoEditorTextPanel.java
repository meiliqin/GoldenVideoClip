package com.standisland.god.demo.feature.sticker;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatDialog;
import androidx.databinding.DataBindingUtil;

import com.standisland.god.demo.R;
import com.standisland.god.demo.common.utils.KeyboardUtil;
import com.standisland.god.demo.feature.sticker.IMG.IMGText;



public class DialogVideoEditorTextPanel extends AppCompatDialog {
    private Activity mContext;
    private IMGText mIMGText;
    private int mFromPage;
    private VideoEditorTextPanel panelText;
    private View bottom;
    public DialogVideoEditorTextPanel(Activity activity, int fromPage) {
        super(activity, R.style.GlDialog);
        mContext = activity;
        this.mFromPage = fromPage;
        initView();
    }


    public void setTextEditCallback(TextEditCallback textEditCallback) {
        panelText.setTextCallback(textEditCallback);
    }

    public void setIMGText(IMGText IMGText) {
        mIMGText = IMGText;
    }

    private void initView() {
        setContentView(R.layout.dialog_editor_text_panel);
        panelText=findViewById(R.id.panel_text);
        bottom=findViewById(R.id.bottom);
        Window window = getWindow();
        if (window != null) {
            window.setGravity(Gravity.BOTTOM);
            WindowManager.LayoutParams lp = window.getAttributes();
            window.setWindowAnimations(R.style.bottomDialogStyle);
            lp.width = WindowManager.LayoutParams.MATCH_PARENT;
            lp.height = WindowManager.LayoutParams.MATCH_PARENT;
            window.setAttributes(lp);
        }
        setCancelable(true);
        setCanceledOnTouchOutside(false);
       panelText.setDialog(this);
      panelText.setmFromPage(mFromPage);
        KeyboardUtils.registerSoftInputChangedListener(mContext, new KeyboardUtils.OnSoftInputChangedListener() {
            @Override
            public void onSoftInputChanged(int height) {
                Log.d("registerSoftInput",height+"");
                ViewGroup.LayoutParams layoutParams = bottom.getLayoutParams();
                if(layoutParams!=null && layoutParams.height != height){
                    layoutParams.height = height;
                    bottom.setLayoutParams(layoutParams);
                }
            }
        });
    }
    @Override
    protected void onStart() {
        super.onStart();
       panelText.setTextSticker(mIMGText);
    }

    @Override
    public void dismiss() {
        super.dismiss();
        KeyboardUtil.hideKeyboard(panelText);
    }

    @Override
    public void show() {
        super.show();
        KeyboardUtil.showKeyboard(panelText.getEtContent());
    }

    public Context getActivity() {
        return mContext;
    }
}
