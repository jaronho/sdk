package com.yaxon.hudmain.jh.utils;

import java.util.ArrayList;
import java.util.Calendar;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.util.Log;

public class NotifyCenter {
	private static Activity mActivity = null;
	private static Class<?> mClass = null;
	private static ArrayList<PendingInfo> mPendingList = new ArrayList<PendingInfo>();
	
	public static void regist(Activity activity, Class<?> cls) {
		mActivity = activity;
		mClass = cls;
	}
	
	public static void add(int type, int key, String title, String text, long delay) {
		if (null == mActivity || null == mClass) {
			Log.e("cocos2d", "NotifyCenter -> add -> must do regist first");
			return;
		}
		Log.e("cocos2d", "NotifyCenter -> add -> type: " + type + ", key: " + key + ", title: " + title + ", text: " + text + ", delay: " + delay);
		Intent intent = new Intent(mActivity, mClass);
		intent.putExtra("type", type);
		intent.putExtra("key", key);
		intent.putExtra("title", title);
		intent.putExtra("text", text);
		AlarmManager alarmMgr = (AlarmManager)mActivity.getSystemService(Activity.ALARM_SERVICE);
		PendingIntent pi = PendingIntent.getBroadcast(mActivity, key, intent, PendingIntent.FLAG_UPDATE_CURRENT);
		if (alarmMgr.equals(pi)) {
			return;
		}
		switch (type) {
		case 1:		// delay from current time
			Log.e("cocos2d", "NotifyCenter -> add -> delay from current time");
			alarmMgr.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + delay*1000, pi);
			break;
		case 2:		// every day fixed time
			int hour = (int)(delay/3600);
			if (hour >= 24) {
				hour = 23;
			}
			delay = delay - hour*3600;
			int minute = (int)(delay/60);
			if (minute >= 60) {
				minute = 59;
			}
			delay = delay - minute*60;
			int seconds = (int)delay;
			if (seconds > 60) {
				seconds = 60;
			}
			Log.e("cocos2d", "NotifyCenter -> add -> every day fixed time: " + hour + ":" + minute + ":" + seconds);
			Calendar calendar = Calendar.getInstance();
			calendar.setTimeInMillis(System.currentTimeMillis());
			calendar.set(Calendar.HOUR_OF_DAY, hour);
			calendar.set(Calendar.MINUTE, minute);
			calendar.set(Calendar.SECOND, seconds);
			calendar.set(Calendar.MILLISECOND, 0);
			alarmMgr.setRepeating(AlarmManager.RTC_WAKEUP, calendar.getTimeInMillis(), AlarmManager.INTERVAL_DAY, pi);
			break;
		}
		PendingInfo info = new PendingInfo();
		info.type = type;
		info.key = key;
		info.pi = pi;
		mPendingList.add(info);
	}
	
	public static void removeType(int type) {
		if (null == mActivity || null == mClass) {
			Log.e("cocos2d", "NotifyCenter -> removeType -> must do regist first");
			return;
		}
		Log.e("cocos2d", "NotifyCenter -> removeType -> type: " + type);
		AlarmManager alarmMgr = (AlarmManager)mActivity.getSystemService(Activity.ALARM_SERVICE);
		for (PendingInfo info : mPendingList) {
			if (type == info.type && null != info.pi) {
				alarmMgr.cancel(info.pi);
				mPendingList.remove(info);
			}
		}
	}
	
	public static void removeKey(int key) {
		if (null == mActivity || null == mClass) {
			Log.e("cocos2d", "NotifyCenter -> removeKey -> must do regist first");
			return;
		}
		Log.e("cocos2d", "NotifyCenter -> removeKey -> key: " + key);
		AlarmManager alarmMgr = (AlarmManager)mActivity.getSystemService(Activity.ALARM_SERVICE);
		for (PendingInfo info : mPendingList) {
			if (key == info.key && null != info.pi) {
				alarmMgr.cancel(info.pi);
				mPendingList.remove(info);
			}
		}
	}
	
	public static void clear() {
		if (null == mActivity || null == mClass) {
			Log.e("cocos2d", "NotifyCenter -> clear -> must do regist first");
			return;
		}
		Log.e("cocos2d", "NotifyCenter -> clear");
		AlarmManager alarmMgr = (AlarmManager)mActivity.getSystemService(Activity.ALARM_SERVICE);
		for (PendingInfo info : mPendingList) {
			if (null != info.pi) {
				alarmMgr.cancel(info.pi);
			}
		}
		mPendingList.clear();
	}
	
	private static class PendingInfo {
		public int type = 0;
		public int key = 0;
		public PendingIntent pi = null;
	}
}
