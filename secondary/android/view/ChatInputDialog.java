package com.jaronho.sdk.utils.view;

import android.app.Activity;
import android.app.Dialog;
import android.graphics.Point;
import android.view.Gravity;
import android.view.WindowManager;

import com.jaronho.sdk.utils.SoftKeyboard;

/**
 * Author:  jaron.ho
 * Date:    2017-04-30
 * Brief:   聊天输入框
 */

public class ChatInputDialog extends Dialog {
    /* themeResId,范例:
        <style name="dialog_chat_input">
            <item name="android:windowFrame">@null</item>
            <item name="android:windowIsFloating">true</item>
            <item name="android:windowIsTranslucent">false</item>
            <item name="android:windowFullscreen">false</item>
            <item name="android:windowNoTitle">true</item>
            <item name="android:background">#00000000</item>
            <item name="android:windowBackground">@android:color/transparent</item>
            <item name="android:backgroundDimEnabled">false</item>
        </style>
    */
    public ChatInputDialog(Activity activity, int themeResId, int layoutId) {
        super(activity, themeResId);
        setContentView(layoutId);
        getWindow().setGravity(Gravity.CENTER_HORIZONTAL|Gravity.BOTTOM);
        Point displaySize = new Point();
        activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
        WindowManager.LayoutParams lp = getWindow().getAttributes();
        lp.width = displaySize.x;
        getWindow().setAttributes(lp);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        new SoftKeyboard(getWindow().getDecorView()) {
            @Override
            public void onSoftKeyboardOpened(int keyboardHeightInPx) {
            }
            @Override
            public void onSoftKeyboardClosed() {
                dismiss();
            }
        }.toogle();
    }
}
