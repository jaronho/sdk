package com.jaronho.sdk.utils;

import java.util.Timer;
import java.util.TimerTask;

/**
 * Author:  jaron.ho
 * Date:    2017-05-22
 * Brief:   日历监听器
 */

public abstract class QuickCalendarListener {
    private QuickCalendar mQuickCalendar = null;
    private Timer mTimer = null;        // 定时器
    private int mLastMinute = 0;        // 跨分钟计算
    private int mLastHour = 0;          // 跨小时计算
    private int mLastDay = 0;           // 跨天计算
    private int mLastWeekDay = 0;       // 跨周计算
    private int mLastMonth = 0;         // 跨月计算
    private int mLastYear = 0;          // 跨年计算

    /**
     * 功  能: 构造函数
     * 参  数: year - 年
     *         month - 月
     *         day - 日
     *         hour - 时
     *         minute - 分
     *         second - 秒
     * 返回值: 无
     */
    public QuickCalendarListener(int year, int month, int day, int hour, int minute, int second) {
        mQuickCalendar = new QuickCalendar(year, month, day, hour, minute, second);
    }

    /**
     * 功  能: 构造函数
     * 参  数: timeStamp - 时间戳(毫秒)
     * 返回值: 无
     */
    public QuickCalendarListener(long timeStamp) {
        mQuickCalendar = new QuickCalendar(timeStamp);
    }

    /**
     * 功  能: 构造函数
     * 参  数: 无
     * 返回值: 无
     */
    public QuickCalendarListener() {
        mQuickCalendar = new QuickCalendar();
    }

    @Override
    protected void finalize() throws Throwable {
        stop();
        super.finalize();
    }

    /**
     * 功  能: 开始监听(每秒监听)
     * 参  数: interval - 时间间隔(毫秒)
     * 返回值: 无
     */
    public void start(final int interval) {
        if (interval <= 0) {
            throw new AssertionError("interval must > 0");
        }
        stop();
        mLastMinute = mQuickCalendar.getMinute();
        mLastHour = mQuickCalendar.getHour();
        mLastDay = mQuickCalendar.getDay();
        mLastWeekDay = mQuickCalendar.getDayOfWeek();
        mLastMonth = mQuickCalendar.getMonth();
        mLastYear = mQuickCalendar.getYear();
        mTimer = new Timer();
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mQuickCalendar.setTime(getTimeStamp());
                int currentMinute = mQuickCalendar.getMinute();
                int currentHour = mQuickCalendar.getHour();
                int currentDay = mQuickCalendar.getDay();
                int currentWeekDay = mQuickCalendar.getDayOfWeek();
                int currentMonth = mQuickCalendar.getMonth();
                int currentYear = mQuickCalendar.getYear();
                if (currentMinute != mLastMinute || interval >= 60000) {    // 跨分钟
                    mLastMinute = currentMinute;
                    onNewMinute(mQuickCalendar);
                }
                if (currentHour != mLastHour || interval >= 3600000) {    // 跨小时
                    mLastHour = currentHour;
                    onNewHour(mQuickCalendar);
                }
                if (currentDay != mLastDay || interval >= 86400000) {      // 跨天
                    mLastDay = currentDay;
                    onNewDay(mQuickCalendar);
                }
                if (currentWeekDay != mLastWeekDay || interval >= 604800000) {  // 跨周
                    mLastWeekDay = currentWeekDay;
                    onNewWeek(mQuickCalendar);
                }
                if (currentMonth != mLastMonth) {  // 跨月
                    mLastMonth = currentMonth;
                    onNewMonth(mQuickCalendar);
                }
                if (currentYear != mLastYear) {    // 跨年
                    mLastYear = currentYear;
                    onNewYear(mQuickCalendar);
                }
                onInterval(mQuickCalendar);
            }
        }, 0, interval);
    }

    /**
     * 功  能: 停止监听
     * 参  数: 无
     * 返回值: 无
     */
    public void stop() {
        if (null != mTimer) {
            mTimer.cancel();
            mTimer = null;
        }
    }

    /**
     * 功  能: 获取时间戳(内部调用,可重写)
     * 参  数: 无
     * 返回值: long,时间戳
     */
    protected long getTimeStamp() {
        return System.currentTimeMillis();
    }

    /**
     * 功  能: 触发新的一分钟
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewMinute(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发新的一小时
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewHour(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发新的一天
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewDay(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发新的一周
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewWeek(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发新的一月
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewMonth(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发新的一年
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onNewYear(QuickCalendar quickCalendar) {
    };

    /**
     * 功  能: 触发时间间隔
     * 参  数: quickCalendar - 日历
     * 返回值: 无
     */
    protected void onInterval(QuickCalendar quickCalendar) {
    };
}
