package com.jaronho.sdk.utils;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Environment;
import android.os.Looper;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.Thread.UncaughtExceptionHandler;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

/**
 * Author:  jaron.ho
 * Date:    2017-05-23
 * Brief:   当程序发生Uncaught异常时,由该类来接管程序,并记录错误信息到日志文件
 */

public class CrashTracker implements UncaughtExceptionHandler {
	private static final String TAG = CrashTracker.class.getSimpleName();
	private static CrashTracker mInstance = null;
	private Context mContext = null;
	private UncaughtExceptionHandler mDefaultHandler = null;	// 系统默认的UncaughtException处理类
	private Map<String, String> mInfos = new HashMap<>();	// 用来存储设备信息和异常信息
	private String mFilePath = "";	// 日志文件保存路径
	private String mFilePrefix = "";	// 日志文件名前缀
	private String mToastTips = "";	// 错误提示信息
	private Handler mHandler = null;	// 处理句柄

	public static abstract class Handler {
		/**
		 * 功  能: 处理回调
		 * 参  数: filePath - 日志文件保存路径
		 * 		   filename - 日志文件名
		 * 		   content - 日志内容
		 * 返回值: 无
		 */
		public abstract void onCallback(String filePath, String filename, String content);
	}

	/**
	 * 功  能: 构造函数,声明为私有保证只有一个CrashTracker实例
	 * 参  数: 无
	 * 返回值: 无
	 */
	private CrashTracker() {}

	/**
	 * 功  能: 获取单例
	 * 参  数: 无
	 * 返回值: CrashTracker
	 */
	public static CrashTracker getInstance() {
		if (null == mInstance) {
			synchronized (CrashTracker.class) {
				if (null == mInstance) {
					mInstance = new CrashTracker();
				}
			}
		}
		return mInstance;
	}

	/**
	 * 功  能: 运行
	 * 参  数: context - 上下文活动
	 * 		   filePath - 日志文件路径,默认为:外部存储路径+包名,例:"/sdcard/com.demo.app/"
	 * 		   filePrefix - 日志文件名前缀,默认为"crash"
	 * 		   toastTips -  错误提示信息
	 * 		   handler - 错误处理句柄
	 * 返回值: 无
	 */
	public void run(Context context, String filePath, String filePrefix, String toastTips, Handler handler) {
		if (null != mContext) {
			throw new AssertionError("crash tracker is in running");
		}
		mContext = context;
		mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();	// 获取系统默认的UncaughtException处理器
		if (null == filePath || filePath.isEmpty()) {
			filePath = Environment.getExternalStorageDirectory().getPath() + "/" + context.getPackageName();
		}
		if (!filePath.endsWith("/")) {
			filePath += "/";
		}
		mFilePath = filePath;
		mFilePrefix = (null == filePrefix || filePrefix.isEmpty()) ? "crash" : filePrefix;
		mToastTips = (null == toastTips) ? "" : toastTips;
		mHandler = handler;
		Thread.setDefaultUncaughtExceptionHandler(this);	// 设置程序的默认处理器
		Log.d(TAG, "file path: " + mFilePath + ", toast tips: " + toastTips);
	}

	/**
	 * 功  能: 运行
	 * 参  数: context - 上下文活动
	 * 		   toastTips -  错误提示信息
	 * 		   handler - 错误处理句柄
	 * 返回值: 无
	 */
	public void run(Context context, String toastTips, Handler handler) {
		run(context, "", "", toastTips, handler);
	}

	/**
	 * 功  能: 运行
	 * 参  数: context - 上下文活动
	 * 		   handler - 错误处理句柄
	 * 返回值: 无
	 */
	public void run(Context context, Handler handler) {
		run(context, "", "", "", handler);
	}

	/**
	 * 功  能: 运行
	 * 参  数: context - 上下文活动
	 * 返回值: 无
	 */
	public void run(Context context) {
		run(context, "", "", "", null);
	}

	/**
	 * 功  能: 当UncaughtException发生时会转入该函数来处理
	 * 参  数: thread - 异常线程
	 * 		   ex - 错误
	 * 返回值: 无
	 */
	@Override
	public void uncaughtException(Thread thread, Throwable ex) {
		if (!handleException(ex) && null != mDefaultHandler) {
			mDefaultHandler.uncaughtException(thread, ex);	// 如果用户没有处理则让系统默认的异常处理器来处理
		} else {
			try {
				Thread.sleep(3000);
			} catch (InterruptedException e) {
				Log.e(TAG, "error", e);
			}
			android.os.Process.killProcess(android.os.Process.myPid());	// 退出程序
			System.exit(1);
		}
	}

	/**
	 * 功  能: 自定义错误处理,收集错误信息,发送错误报告等操作均在此完成
	 * 参  数: ex - 错误
	 * 返回值: boolean,true:处理了该异常信息,false:未处理
	 */
	private boolean handleException(final Throwable ex) {
		if (null == ex) {
			return false;
		}
		new Thread() {
			@Override
			public void run() {	// 使用Toast来显示异常信息
				Looper.prepare();
				Toast.makeText(mContext, mToastTips.isEmpty() ? ex.getMessage() : mToastTips, Toast.LENGTH_LONG).show();
				Looper.loop();
			}
		}.start();
		collectDeviceInfo();	// 收集设备参数信息
		writeInfoToFile(ex);	// 保存日志文件
		return true;
	}

	/**
	 * 功  能: 收集设备参数信息
	 * 参  数: 无
	 * 返回值: 无
	 */
	private void collectDeviceInfo() {
		String devicdInfo = "";
		try {
			PackageManager pm = mContext.getPackageManager();
			PackageInfo pi = pm.getPackageInfo(mContext.getPackageName(), PackageManager.GET_ACTIVITIES);
			if (null != pi) {
				String versionName = (null == pi.versionName) ? "null" : pi.versionName;
				String versionCode = String.valueOf(pi.versionCode);
				mInfos.put("version_name", versionName);
				mInfos.put("version_code", versionCode);
				devicdInfo += "\nversion_name=" + versionName;
				devicdInfo += "\nversion_code=" + versionCode;
			}
		} catch (NameNotFoundException e) {
			Log.d(TAG, "information ==========>>>>>" + devicdInfo + "\n<<<<<==============================");
			Log.e(TAG, "an error occured when collect package info", e);
		}
		Field[] fields = Build.class.getDeclaredFields();
		for (Field field : fields) {
			try {
				field.setAccessible(true);
				String name = field.getName();
				String value = field.get(null).toString();
				mInfos.put(name, value);
				devicdInfo += "\n" + name + "=" + value;
			} catch (Exception e) {
				Log.d(TAG, "information ==========>>>>>" + devicdInfo + "\n<<<<<==============================");
				Log.e(TAG, "an error occured when collect crash info", e);
			}
		}
	}

	/**
	 * 功  能: 保存信息到文件中
	 * 参  数: ex - 错误
	 * 返回值: 无
	 */
	private void writeInfoToFile(Throwable ex) {
		StringBuilder stringBuilder = new StringBuilder();
		for (Map.Entry<String, String> entry : mInfos.entrySet()) {
			stringBuilder.append(entry.getKey());
			stringBuilder.append("=");
			stringBuilder.append(entry.getValue());
			stringBuilder.append("\n");
		}
		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		ex.printStackTrace(printWriter);
		printWriter.close();
		stringBuilder.append(stringWriter.toString());
		String crashInfo = "";
		String filename = "";
		try {
			if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
				File dir = new File(mFilePath);
				if (dir.exists() || dir.mkdirs()) {
					String date = new SimpleDateFormat("yyyyMMdd-HHmmss-SSS", Locale.getDefault()).format(new Date());
					filename = mFilePrefix + "-" + date + ".log";
					FileOutputStream fos = new FileOutputStream(mFilePath + filename);
					crashInfo = stringBuilder.toString();
					fos.write(crashInfo.getBytes());
					fos.close();
					Log.d(TAG, "information ==========>>>>>\n" + crashInfo + "<<<<<==============================");
					Log.d(TAG, "crash filename: " + filename);
				}
			}
		} catch (Exception e) {
			Log.d(TAG, "information ==========>>>>>\n" + crashInfo + "<<<<<==============================");
			Log.e(TAG, "an error occured while writting file...", e);
		}
		if (null != mHandler) {
			mHandler.onCallback(mFilePath, filename, stringBuilder.toString());
		}
	}
}
