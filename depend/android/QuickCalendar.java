package com.jaronho.sdk.utils;

import java.util.Calendar;

/**
 * Author:  jaron.ho
 * Date:    2017-05-22
 * Brief:   日历
 */

public class QuickCalendar {
    private Calendar mCalendar = Calendar.getInstance();

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
    public QuickCalendar(int year, int month, int day, int hour, int minute, int second) {
        setDate(year, month, day, hour, minute, second);
    }

    /**
     * 功  能: 构造函数
     * 参  数: timeStamp - 时间戳(毫秒)
     * 返回值: 无
     */
    public QuickCalendar(long timeStamp) {
        setTime(timeStamp);
    }

    /**
     * 功  能: 构造函数
     * 参  数: 无
     * 返回值: 无
     */
    public QuickCalendar() {
        setNow();
    }

    /**
     * 功  能: 设置指定日期
     * 参  数: year - 年
     *         month - 月
     *         day - 日
     *         hour - 时
     *         minute - 分
     *         second - 秒
     * 返回值: 无
     */
    public void setDate(int year, int month, int day, int hour, int minute, int second) {
        mCalendar.clear();
        mCalendar.set(year, month, day, hour, minute, second);
    }

    /**
     * 功  能: 设置指定时间
     * 参  数: timeStamp - 时间戳(毫秒)
     * 返回值: 无
     */
    public void setTime(long timeStamp) {
        mCalendar.clear();
        mCalendar.setTimeInMillis(timeStamp);
    }

    /**
     * 功  能: 回到当前时间
     * 参  数: 无
     * 返回值: 无
     */
    public void setNow() {
        setTime(System.currentTimeMillis());
    }

    /**
     * 功  能: 获取当前时间戳
     * 参  数: 无
     * 返回值: long,毫秒
     */
    public long getTimeStamp() {
        return mCalendar.getTimeInMillis();
    }

    /**
     * 功  能: 获取年
     * 参  数: 无
     * 返回值: int
     */
    public int getYear() {
        return mCalendar.get(Calendar.YEAR);
    }

    /**
     * 功  能: 获取月
     * 参  数: 无
     * 返回值: int,一月:0,二月:1,三月:2,四月:3,五月:4,六月:5,七月:6,八月:7,九月:8,十月:09,十一月:10,十二月:11
     */
    public int getMonth() {
        return mCalendar.get(Calendar.MONTH);
    }

    /**
     * 功  能: 获取日
     * 参  数: 无
     * 返回值: int
     */
    public int getDay() {
        return mCalendar.get(Calendar.DATE);
    }

    /**
     * 功  能: 获取小时
     * 参  数: 无
     * 返回值: int
     */
    public int getHour() {
        return mCalendar.get(Calendar.HOUR_OF_DAY);
    }

    /**
     * 功  能: 获取分钟
     * 参  数: 无
     * 返回值: int
     */
    public int getMinute() {
        return mCalendar.get(Calendar.MINUTE);
    }

    /**
     * 功  能: 获取秒
     * 参  数: 无
     * 返回值: int
     */
    public int getSecond() {
        return mCalendar.get(Calendar.SECOND);
    }

    /**
     * 功  能: 获取毫秒
     * 参  数: 无
     * 返回值: int
     */
    public int getMillisecond() {
        return mCalendar.get(Calendar.MILLISECOND);
    }

    /**
     * 功  能: 获取当前在本年第几周
     * 参  数: 无
     * 返回值: int
     */
    public int getWeekOfYear() {
        return mCalendar.get(Calendar.WEEK_OF_YEAR);
    }

    /**
     * 功  能: 获取当前在本月第几周
     * 参  数: 无
     * 返回值: int
     */
    public int getWeekOfMonth() {
        return mCalendar.get(Calendar.WEEK_OF_MONTH);
    }

    /**
     * 功  能: 获取当前在本年第几天
     * 参  数: 无
     * 返回值: int
     */
    public int getDayOfYear() {
        return mCalendar.get(Calendar.DAY_OF_YEAR);
    }

    /**
     * 功  能: 获取当前在本周第几天
     * 参  数: 无
     * 返回值: int,周天:1,周一:2,周二:3,周三:4,周四:5,周五:6,周六:7
     */
    public int getDayOfWeek() {
        return mCalendar.get(Calendar.DAY_OF_WEEK);
    }
}
