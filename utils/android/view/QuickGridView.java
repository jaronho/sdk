package com.jaronho.sdk.utils.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.GridView;

/**
 * Author:  jaron.ho
 * Date:    2017-04-06
 * Brief:   QuickGridView
 */

public class QuickGridView extends GridView {
    public QuickGridView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int expandSpec = MeasureSpec.makeMeasureSpec(Integer.MAX_VALUE >> 2, MeasureSpec.AT_MOST);
        super.onMeasure(widthMeasureSpec, expandSpec);
    }
}
