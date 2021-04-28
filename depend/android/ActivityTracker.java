package com.jaronho.sdk.utils;

import android.app.Activity;
import android.app.Application;
import android.app.Application.ActivityLifecycleCallbacks;
import android.content.Context;
import android.os.Bundle;

import java.util.ArrayList;
import java.util.List;

/**
 * Author:  Administrator
 * Date:    2017/5/8
 * Brief:   ActivityTracker
 */

public class ActivityTracker implements ActivityLifecycleCallbacks {
    private static ActivityTracker mInstance = null;
    private static Application mApp = null;
    private static List<Activity> mActivityList = new ArrayList<>();
    private static int mActivityCount = 0;
    private static boolean mIsForground = false;
    private static Activity mTopActivity = null;

    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
        if (!mActivityList.equals(activity)) {
            mActivityList.add(activity);
        }
    }

    @Override
    public void onActivityStarted(Activity activity) {
        ++mActivityCount;
    }

    @Override
    public void onActivityResumed(Activity activity) {
        mIsForground = true;
        mTopActivity = activity;
    }

    @Override
    public void onActivityPaused(Activity activity) {
    }

    @Override
    public void onActivityStopped(Activity activity) {
        --mActivityCount;
        if (0 == mActivityCount) {
            mIsForground = false;
        }
    }

    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
    }

    @Override
    public void onActivityDestroyed(Activity activity) {
        if (mActivityList.equals(activity)) {
            mActivityList.remove(activity);
        }
    }

    /**
     * 功  能: 初始化
     * 参  数: app - 应用对象
     * 返回值: 无
     */
    public static void init(Application app) {
        if (null == mInstance) {
            synchronized (ActivityTracker.class) {
                mInstance = new ActivityTracker();
                app.registerActivityLifecycleCallbacks(mInstance);
                mApp = app;
            }
        }
    }

    /**
     * 功  能: 获取应用实例
     * 参  数: 无
     * 返回值: Application
     */
    public static Application getApplication() {
        return mApp;
    }

    /**
     * 功  能: 获取应用上下文
     * 参  数: 无
     * 返回值: Context
     */
    public static Context getApplicationContext() {
        return null != mApp ? mApp.getApplicationContext() : null;
    }

    /**
     * 功  能: 结束活动
     * 参  数: cls - 活动类名
     * 返回值: 无
     */
    public static void finishActivity(Class<? extends Activity> cls) {
        for (int i = 0, len = mActivityList.size(); i < len; ++i) {
            Activity activity = mActivityList.get(i);
            if (null != activity && activity.getClass().equals(cls)) {
                activity.finish();
                return;
            }
        }
    }

    /**
     * 功  能: 获取活动个数
     * 参  数: 无
     * 返回值: int
     */
    public static int getActivityCount() {
        return mActivityCount;
    }

    /**
     * 功  能: 是否在前台
     * 参  数: 无
     * 返回值: boolean
     */
    public static boolean isForground() {
        return mIsForground;
    }

    /**
     * 功  能: 获取顶部活动
     * 参  数: 无
     * 返回值: Activity
     */
    public static Activity getTopActivity() {
        return mTopActivity;
    }
}
