package com.jaronho.sdk.utils;

import android.app.Activity;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.List;

/**
 * Author:  jaron.ho
 * Date:    2017-04-06
 * Brief:   BaseActivity
 */

public abstract class BaseActivity extends AppCompatActivity {
    private static final List<BaseActivity> mActivities = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mActivities.add(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mActivities.remove(this);
    }

    /**
     * 功  能: 退出应用
     * 参  数: 无
     * 返回值: 无
     */
    public void exitApp() {
        List<BaseActivity> tmpActivities;
        synchronized (mActivities) {
            tmpActivities = new ArrayList<>(mActivities);
        }
        for (BaseActivity activity : tmpActivities) {
            if (!activity.isFinishing()) {
                activity.finish();
            }
        }
        android.os.Process.killProcess(android.os.Process.myPid());
    }
}
