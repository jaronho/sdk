package com.jaronho.sdk.utils;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.Context;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings.Secure;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;

/**
 * Author:  jaron.ho
 * Date:    2017-02-08
 * Brief:   工具集
 */
 
public final class Util {

    /**
     * 功  能: 将px值转换为dip或dp值,保证尺寸大小不变
     * 参  数: context - 上下文
     *         pxValue - 像素值
     * 返回值: int
     */
    public static int px2dp(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(pxValue / scale + 0.5f);
    }

    /**
     * 功  能: 将dip或dp值转换为px值,保证尺寸大小不变
     * 参  数: context - 上下文
     *         dpValue - dp值
     * 返回值: int
     */
    public static int dp2px(Context context, float dpValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int)(dpValue * scale + 0.5f);
    }

    /**
     * 功  能: 将px值转换为sp值,保证尺寸大小不变
     * 参  数: context - 上下文
     *         pxValue - 像素值
     * 返回值: int
     */
    public static int px2sp(Context context, float pxValue) {
        final float fontScale = context.getResources().getDisplayMetrics().scaledDensity;
        return (int)(pxValue / fontScale + 0.5f);
    }

    /**
     * 功  能: 将sp值转换为px值,保证尺寸大小不变
     * 参  数: context - 上下文
     *         spValue - sp值
     * 返回值: int
     */
    public static int sp2px(Context context, float spValue) {
        final float fontScale = context.getResources().getDisplayMetrics().scaledDensity;
        return (int)(spValue * fontScale + 0.5f);
    }

    /**
     * 功  能: 获取屏幕像素x
     * 参  数: activity - 活动
     * 返回值: int
     */
    public static int getXScreenPx(Activity activity) {
        DisplayMetrics dm = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(dm);
        return dm.widthPixels;
    }

    /**
     * 功  能: 获取屏幕像素y
     * 参  数: activity - 活动
     * 返回值: int
     */
    public static int getYScreenPx(Activity activity) {
        DisplayMetrics dm = new DisplayMetrics();
        activity.getWindowManager().getDefaultDisplay().getMetrics(dm);
        return dm.heightPixels;
    }

	/**
     * 功  能: 复制字符串到剪贴板
     * 参  数: activity - 活动
	 *		   str - 字符串
     * 返回值: 无
     */
	public static void copyString(final Activity activity, final String str) {
		if (null == activity) {
			return;
		}
		activity.runOnUiThread(new Runnable() {
			@SuppressLint("NewApi")
			@SuppressWarnings("deprecation")
			@Override
			public void run() {
				Object obj = activity.getSystemService(Activity.CLIPBOARD_SERVICE);
				((android.content.ClipboardManager)obj).setText(str);
			}
		});
	}
	
	/**
     * 功  能: 是否有网络
     * 参  数: activity - 活动
     * 返回值: boolean
     */
	public static boolean isNetworkAvailable(Activity activity) {
		if (null == activity) {
			return false;
		}
		ConnectivityManager cm = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		if (null == cm) {
			return false;
		}
		NetworkInfo[] networkInfoList = cm.getAllNetworkInfo();
		if (null != networkInfoList && networkInfoList.length > 0) {
			for (NetworkInfo aNetworkInfoList : networkInfoList) {
				if (NetworkInfo.State.CONNECTED == aNetworkInfoList.getState()) {
					return true;
				}
			}
        }
		return false;
	}
	
	/**
     * 功  能: 是否为wifi网络
     * 参  数: activity - 活动
     * 返回值: boolean
     */
	public static boolean isNetworkWifi(Activity activity) {
		if (null == activity) {
			return false;
		}
		ConnectivityManager cm = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = cm.getActiveNetworkInfo();
		return null != networkInfo && ConnectivityManager.TYPE_WIFI == networkInfo.getType();
    }
	
	/**
     * 功  能: 是否有移动网络
     * 参  数: activity - 活动
     * 返回值: boolean
     */
	public static boolean isNetworkMobile(Activity activity) {
		if (null == activity) {
			return false;
		}
		ConnectivityManager cm = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = cm.getActiveNetworkInfo();
		return null != networkInfo && ConnectivityManager.TYPE_MOBILE == networkInfo.getType();
    }
	
	/**
     * 功  能: GPS是否打开
     * 参  数: activity - 活动
     * 返回值: boolean
     */
	public static boolean isGpsEnabled(Activity activity) {
		if (null == activity) {
			return false;
		}
		LocationManager lm = ((LocationManager)activity.getSystemService(Context.LOCATION_SERVICE));
		List<String> accessibleProviders = lm.getProviders(true);
		return null != accessibleProviders && accessibleProviders.size() > 0;
    }
	
	/**
     * 功  能: 应用是否在前台运行
     * 参  数: activity - 活动
     * 返回值: boolean
     */
	public static boolean isAppOnForeground(Activity activity) {
		if (null == activity) {
			return false;
		}
		ActivityManager am = (ActivityManager)activity.getSystemService(Context.ACTIVITY_SERVICE);
        List<RunningAppProcessInfo> appProcessList = am.getRunningAppProcesses();
        if (null == appProcessList) {
            return false;
        }
        for (RunningAppProcessInfo appProcess : appProcessList) {
            if (RunningAppProcessInfo.IMPORTANCE_FOREGROUND == appProcess.importance && appProcess.processName.equals(activity.getPackageName())) {
                return true;
            }
        }
        return false;
	}
	
	/**
     * 功  能: 获取MAC地址
     * 参  数: activity - 活动
     * 返回值: String
     */
	public static String getMacAddress(Activity activity) {
		if (null == activity) {
			return "";
		}
		String macAddress = ((WifiManager)activity.getSystemService(Context.WIFI_SERVICE)).getConnectionInfo().getMacAddress();
		if (null == macAddress) {
			return "";
		}
		return macAddress;
	}

	/**
     * 功  能: 获取主机IP地址
     * 参  数: activity - 活动
     * 返回值: String
     */
	public static String getHostIpAddress(Activity activity) {
		if (null == activity) {
			return "";
		}
        WifiManager wifiMgr = (WifiManager)activity.getSystemService(Context.WIFI_SERVICE);
        WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
        int ip = wifiInfo.getIpAddress();
        return ((ip & 0xFF) + "." + ((ip >>>= 8) & 0xFF) + "." + ((ip >>>= 8) & 0xFF) + "." + ((ip >>>= 8) & 0xFF));
    }
	
	/**
     * 功  能: 获取设备id
     * 参  数: activity - 活动
     * 返回值: String
     */
	public static String getDeviceId(Activity activity) {
		if (null == activity) {
			return "";
		}
		return Secure.getString(activity.getContentResolver(), Secure.ANDROID_ID);
	}
	
	/**
     * 功  能: 获取版本名称
     * 参  数: activity - 活动
     * 返回值: String
     */
	public static String getVersionName(Activity activity) {
		if (null == activity) {
			return "";
		}
		try {
			return activity.getPackageManager().getPackageInfo(activity.getPackageName(), 0).versionName;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
	}
	
	/**
     * 功  能: 获取SIM卡类型
     * 参  数: activity - 活动
     * 返回值: String
     */
	public static String getSIM(Activity activity) {
		if (null == activity) {
			return "";
		}
		TelephonyManager tm = (TelephonyManager)activity.getSystemService(Context.TELEPHONY_SERVICE);
		if (TelephonyManager.SIM_STATE_READY != tm.getSimState()) {
			return "SIM_NULL";		// Null
		}
        String operator = tm.getSimOperator();
		switch (operator) {
			case "46000":
			case "46002":
			case "46007":
				return "SIM_YD";        // China Mobile
			case "46001":
			case "46006":
				return "SIM_LT";        // China Unicom
			case "46003":
			case "46005":
				return "SIM_DX";        // China Telecom
		}
    	return "SIM_UNKNOWN";		// Unknown
	}
}
