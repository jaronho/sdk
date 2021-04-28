package com.jaronho.sdk.utils.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ListView;

/**
 * Author:  jaron.ho
 * Date:    2017-04-06
 * Brief:   QuickListView
 */

public class QuickListView extends ListView {
    public QuickListView(Context context) {
        super(context);
    }

    public QuickListView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public QuickListView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int expandSpec = MeasureSpec.makeMeasureSpec(Integer.MAX_VALUE >> 2, MeasureSpec.AT_MOST);
        super.onMeasure(widthMeasureSpec, expandSpec);
    }

    public void performItemClick(int position) {
        int lastPosition = getLastVisiblePosition();
        if (position >= 0 && position <= lastPosition) {
            setSelection(position);
            performItemClick(getChildAt(position), position, getItemIdAtPosition(position));
        }
    }
}
