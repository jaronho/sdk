package com.jaronho.sdk.utils;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.GridView;

/**
 * Author:  jaron.ho
 * Date:    2017-04-06
 * Brief:   GridViewEx
 */

public class GridViewEx extends GridView {
    public GridViewEx(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int expandSpec = MeasureSpec.makeMeasureSpec(Integer.MAX_VALUE >> 2, MeasureSpec.AT_MOST);
        super.onMeasure(widthMeasureSpec, expandSpec);
    }
}
