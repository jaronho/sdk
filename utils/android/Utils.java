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
 * Brief:   工具集
 */

public final class Utils {
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
    public static void startBlink(final View view, final int showTime, final int hideTime, final int count, final Timer.OverHandler handler) {
        if (null != view) {
            if (View.NO_ID == view.getId()) {
                view.setId(View.generateViewId());
            }
            TimerManager.getInstance().run(showTime, count * 2, new Timer.RunHandler() {
                @Override
                public void onCallback(Timer tm, int runCount, Object param) {
                    if (0 == runCount) {
                        view.setVisibility(View.VISIBLE);
                    } else {
                        boolean showFlag = !(boolean) param;
                        tm.setParam(showFlag);
                        tm.setInterval(showFlag ? showTime : hideTime);
                        view.setVisibility(showFlag ? View.VISIBLE : View.INVISIBLE);
                    }
                }
            }, handler, true, true, "TimerBlink_" + view.getId());
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
     * 返回值: 无
     */
    public static void stopBlink(View view) {
        if (null != view) {
            TimerManager.getInstance().stop("TimerBlink_" + view.getId(), true);
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
