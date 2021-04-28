package com.jaronho.sdk.utils;

import android.content.Context;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

/**
 * Author:  jaron.ho
 * Date:    2017-05-03
 * Brief:   SoftKeyboard
 */

public abstract class SoftKeyboard implements ViewTreeObserver.OnGlobalLayoutListener {
    private static final int PIXEL_OFFSET = 100;
    private View mView = null;
    private boolean mIsOpened = false;
    private int mScreenHeight = 0;
    private InputMethodManager mImm = null;

    public SoftKeyboard(View view) {
        mView = view;
        Point p = new Point();
        ((WindowManager)view.getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getSize(p);
        mScreenHeight = p.y;
        mImm = (InputMethodManager)view.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
    }

    @Override
    public void onGlobalLayout() {
        Rect r = new Rect();
        // r will be populated with the coordinates of your view that area still visible.
        mView.getWindowVisibleDisplayFrame(r);
        int heightDiff = mScreenHeight - (r.bottom - r.top);
        if (!mIsOpened && heightDiff > PIXEL_OFFSET) {  // if more than 100 pixels, its probably a keyboard...
            mIsOpened = true;
            onSoftKeyboardOpened(heightDiff);
        } else if (mIsOpened && heightDiff < PIXEL_OFFSET) {
            mIsOpened = false;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                mView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
            } else {
                mView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
            }
            mImm.hideSoftInputFromWindow(mView.getWindowToken(), 0);
            onSoftKeyboardClosed();
        }
    }

    /**
     * 功  能: 开关
     * 参  数: 无
     * 返回值: 无
     */
    public void toogle() {
        if (!mIsOpened) {
            mView.getViewTreeObserver().addOnGlobalLayoutListener(this);
        }
        mImm.toggleSoftInputFromWindow(mView.getWindowToken(), InputMethodManager.SHOW_FORCED, 0);
    }

    /**
     * 功  能: 软键盘被打开
     * 参  数: keyboardHeightInPixel - 键盘高度(像素)
     * 返回值: 无
     */
    public abstract void onSoftKeyboardOpened(int keyboardHeightInPixel);

    /**
     * 功  能: 软键盘被关闭
     * 参  数: 无
     * 返回值: 无
     */
    public abstract void onSoftKeyboardClosed();
}
