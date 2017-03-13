package com.jh.utils;

import android.app.Activity;
import android.graphics.drawable.AnimationDrawable;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.widget.ImageView;

import com.yaxon.hudmain.jh.library.timer.Timer;
import com.yaxon.hudmain.jh.library.timer.TimerManager;

/**
 * Author:  jaron.ho
 * Date:    2017-02-08
 * Brief:   视图工具集
 */

public final class ViewUtil {
    private static long mStartTime = 0;     // 时间消耗(毫秒)

    /**
     * 功  能: 耗时开始,与接口costTimeCalc配对使用
     * 参  数: 无
     * 返回值: 无
     */
    public static void costTimeBegin() {
        mStartTime = SystemClock.elapsedRealtime();
    }

    /**
     * 功  能: 耗时计算,与接口costTimeBegin配对使用
     * 参  数: resetFlag - 耗时是否复位
     *         tag - 打印信息的标签,当为null或为空字符串时,不打印
     *         header - 打印信息的头部,当为null或为空字符串时,不打印
     * 返回值: long,耗时(毫秒)
     */
    public static long costTimeCalc(boolean resetFlag, String tag, String header) {
        long deltaTime = 0;
        if (mStartTime > 0) {
            long currentTime = SystemClock.elapsedRealtime();
            deltaTime = currentTime - mStartTime;
            mStartTime = resetFlag ? 0 : currentTime;
            if (null != tag && tag.length() > 0 && null != header && header.length() > 0) {
                Log.d(tag, "[" + header + "] cost time " + deltaTime + " millisconds");
            }
        }
        return deltaTime;
    }

    /**
     * 功  能: 创建帧动画
     * 参  数: activity - 活动
     *         frameIds - 帧图片资源id
     *         durations - 每帧时间间隔(毫秒)
     *         isLoop - 是否循环播放
     * 返回值: AnimationDrawable
     */
    public static AnimationDrawable createFrameDrawable(Activity activity, int[] frameIds, int[] durations, boolean isLoop) {
        AnimationDrawable ani = new AnimationDrawable();
        for (int i = 0, len = frameIds.length; i < len; ++i) {
            ani.addFrame(ContextCompat.getDrawable(activity, frameIds[i]), durations[i]);
        }
        ani.setOneShot(!isLoop);
        return ani;
    }

    /**
     * 功  能: 创建帧动画
     * 参  数: activity - 活动
     *         frameIds - 帧图片资源id
     *         duration - 每帧时间间隔(毫秒)
     *         isLoop - 是否循环播放
     * 返回值: AnimationDrawable
     */
    public static AnimationDrawable createFrameDrawable(Activity activity, int[] frameIds, int duration, boolean isLoop) {
        AnimationDrawable ani = new AnimationDrawable();
        for (int frameId : frameIds) {
            ani.addFrame(ContextCompat.getDrawable(activity, frameId), duration);
        }
        ani.setOneShot(!isLoop);
        return ani;
    }

    /**
     * 功  能: 开始闪烁
     * 参  数: view - 视图
     *         showTime - 显示时间(毫秒)
     *         hideTime - 隐藏时间(毫秒)
     *         count - 闪烁次数,<=0 时一直闪烁
     *         handler - 闪烁结束回调
     * 返回值: 无
     */
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public static void startBlink(final View view, final int showTime, final int hideTime, final int count, final Timer.OverHandler handler) {
        if (null != view) {
            if (View.NO_ID == view.getId()) {
                view.setId(View.generateViewId());
            }
            view.setVisibility(View.VISIBLE);
            TimerManager.getInstance().run(showTime, count * 2, new Timer.RunHandler() {
                private boolean mShowFlag = true;
                @Override
                public void onCallback(Timer tm, int runCount, Object param) {
                    mShowFlag = !mShowFlag;
                    tm.setInterval(mShowFlag ? showTime : hideTime);
                    view.setVisibility(mShowFlag ? View.VISIBLE : (hideTime > 0 ? View.INVISIBLE : View.VISIBLE));
                }
            }, handler, null, false, "TimerBlink_" + view.getId());
        }
    }

    /**
     * 功  能: 开始闪烁
     * 参  数: view - 视图
     *         showTime - 显示时间(毫秒)
     *         hideTime - 隐藏时间(毫秒)
     *         count - 闪烁次数,<=0 时一直闪烁
     *         showAtLast - 当闪烁次数到达后,是否显示(true:显示,false:隐藏)
     * 返回值: 无
     */
    public static void startBlink(final View view, final int showTime, final int hideTime, final int count, final boolean showAtLast) {
        startBlink(view, showTime, hideTime, count, new Timer.OverHandler() {
            @Override
            public void onCallback(Timer tm, Object param) {
                view.setVisibility(showAtLast ? View.VISIBLE : View.INVISIBLE);
            }
        });
    }

    /**
     * 功  能: 停止闪烁
     * 参  数: view - 视图
     *         showAtLast - 是否显示(true:显示,false:隐藏)
     * 返回值: 无
     */
    public static void stopBlink(View view, boolean showAtLast) {
        if (null != view) {
            TimerManager.getInstance().stop("TimerBlink_" + view.getId(), false);
            view.setVisibility(showAtLast ? View.VISIBLE : View.INVISIBLE);
        }
    }

    /**
     * 功  能: 创建环形进度条
     * 参  数: image - 图片
     *         startAngle - 开始角度(顺时针),[0, 360)
     *         sweepAngle - 扫描角度(顺时针),[0, 360]
     * 返回值: SectorDrawable
     */
    public static SectorDrawable createCircleProgress(ImageView image, float startAngle, float sweepAngle) {
        SectorDrawable sectorDrawable = null;
        if (null != image) {
            sectorDrawable = new SectorDrawable(image.getDrawable(), startAngle);
            image.setImageDrawable(sectorDrawable);
            sectorDrawable.setSweepAngle(sweepAngle);
        }
        return sectorDrawable;
    }
}
