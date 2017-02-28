package com.jh.utils;

import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.Context;
import android.location.LocationManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings.Secure;
import android.telephony.TelephonyManager;

public final class Util {
	
	public static void killApp(Activity activity) {
		if (null == activity) {
			return;
		}
		activity.finish();
		android.os.Process.killProcess(android.os.Process.myPid());
	}
	
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
				if (android.os.Build.VERSION.SDK_INT > 11) {
					((android.content.ClipboardManager)obj).setText(str);
				} else {
					((android.text.ClipboardManager)obj).setText(str);
				}
			}
		});
	}
	
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
            for (int i = 0; i < networkInfoList.length; i++) {
                if (NetworkInfo.State.CONNECTED == networkInfoList[i].getState()) {
                    return true;
                }
            }
        }
		return false;
	}
	
	public static boolean isNetworkWifi(Activity activity) {
		if (null == activity) {
			return false;
		}
		ConnectivityManager cm = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = cm.getActiveNetworkInfo();
		return null != networkInfo && ConnectivityManager.TYPE_WIFI == networkInfo.getType();
    }
	
	public static boolean isNetworkMobile(Activity activity) {
		if (null == activity) {
			return false;
		}
		ConnectivityManager cm = (ConnectivityManager)activity.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = cm.getActiveNetworkInfo();
		return null != networkInfo && ConnectivityManager.TYPE_MOBILE == networkInfo.getType();
    }
	
	public static boolean isGpsEnabled(Activity activity) {
		LocationManager lm = ((LocationManager)activity.getSystemService(Context.LOCATION_SERVICE));
		List<String> accessibleProviders = lm.getProviders(true);
		return null != accessibleProviders && accessibleProviders.size() > 0;
    }
	
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
	
	public static String getDeviceId(Activity activity) {
		if (null == activity) {
			return "";
		}
		return Secure.getString(activity.getContentResolver(), Secure.ANDROID_ID);
	}
	
	public static String getBundleVersion(Activity activity) {
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
	
	public static String getSimState(Activity activity) {
		if (null == activity) {
			return "";
		}
		TelephonyManager tm = (TelephonyManager)activity.getSystemService(Context.TELEPHONY_SERVICE);
		if (TelephonyManager.SIM_STATE_READY != tm.getSimState()) {
			return "SIM_NULL";		// Null
		}
        String operator = tm.getSimOperator();
    	if (operator.equals("46000") || operator.equals("46002") || operator.equals("46007")) {
    		return "SIM_YD";		// China Mobile
		} else if (operator.equals("46001") || operator.equals("46006")) {
			return "SIM_LT";		// China Unicom
    	} else if (operator.equals("46003") || operator.equals("46005")) {
    		return "SIM_DX";		// China Telecom
    	}
    	return "SIM_UNKNOWN";		// Unknown
	}
}
