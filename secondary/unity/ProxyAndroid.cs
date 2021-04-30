using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class ProxyAndroid : MonoBehaviour {
	private static Handler mDefaultHandler = null;
	private static Handler mExceptionHandler = null;
	private static Handler mHintHandler = null;
	private static Hashtable mHandlerMap = new Hashtable();

	public delegate void Handler(string jsonStr);

	public class Info {
		public string method;
		public int hint = 0;
	}

	// 调用Android对象方法,className:"com.unity3d.player.UnityPlayer",fieldName:"currentActivity",methodName:"callfunc"
	public static void callMethod(string className, string fieldName, string methodName, string param = "") {
		if (RuntimePlatform.Android == Application.platform) {
			try {
				using (AndroidJavaClass jc = new AndroidJavaClass(className)) {
					using (AndroidJavaObject jo = jc.GetStatic<AndroidJavaObject>(fieldName)) {
						jo.Call(methodName, param);
					}
				}
			} catch (System.Exception e) {
				print(className + " => " + methodName + " => " + param + "\n" + e.Message);
				if (null != mExceptionHandler) {
					mExceptionHandler(className + " => " + methodName + " => " + param + "\n" + e.Message);
				}
			}
		}
	}

	// 调用Android静态方法,className:"com.unity3d.player.UnityPlayer",methodName:"callfunc"
	public static void callStaticMethod(string className, string methodName, string param = "") {
		if (RuntimePlatform.Android == Application.platform) {
			try {
				using (AndroidJavaClass jc = new AndroidJavaClass(className)) {
					jc.CallStatic(methodName, param);
				}
			} catch (System.Exception e) {
				print(className + " => " + methodName + " => " + param + "\n" + e.Message);
				if (null != mExceptionHandler) {
					mExceptionHandler(className + " => " + methodName + " => " + param + "\n" + e.Message);
				}
			}
		}
	}

	// 设置默认处理句柄
	public static void setDefaultHandler(Handler handler) {
		mDefaultHandler = handler;
	}

	// 设置异常处理句柄
	public static void setExceptionHandler(Handler handler) {
		mExceptionHandler = handler;
	}

	// 设置提示处理句柄
	public static void setHintHandler(Handler handler) {
		mHintHandler = handler;
	}

	// 添加处理句柄
	public static void addHandler(string method, Handler handler) {
		List<Handler> handlers = (List<Handler>)mHandlerMap[method];
		if (null == handlers) {
			handlers = new List<Handler>();
			mHandlerMap.Add(method, handlers);
		}
		if (null != handler) {
			handlers.Add(handler);
		}
	}

	// 删除处理句柄
	public static void delHandler(string method, Handler handler = null) {
		List<Handler> handlers = (List<Handler>)mHandlerMap[method];
		if (null != handlers) {
			if (null == handler) {
				mHandlerMap.Remove(method);
			} else {
				handlers.Remove(handler);
				if (0 == handlers.Count) {
					mHandlerMap.Remove(method);
				}
			}
		}
	}

	// 触发处理句柄(在Java代码中调用,不可为静态函数),jsonStr格式为:{"method":"",...}
	public void onHandler(string jsonStr) {
		try {
			Info info = JsonUtility.FromJson<Info>(jsonStr);
			List<Handler> handlers = (List<Handler>)mHandlerMap[info.method];
			if (null != handlers) {
				if (null != mHintHandler && info.hint > 0) {
					mHintHandler(jsonStr);
				}
				handlers.ForEach(delegate(Handler handler) {
					handler(jsonStr);
				});
			} else {	// 没有处理句柄,使用默认处理句柄
				if (null != mDefaultHandler) {
					mDefaultHandler(jsonStr);
				} else {
					print("can't find handler for '" + info.method + "'");
				}
			}
		} catch (System.ArgumentException e) {
			print("ArgumentException\n" + jsonStr + "\n" + e.Message);
			if (null != mExceptionHandler) {
				mExceptionHandler("ArgumentException\n" + jsonStr + "\n" + e.Message);
			}
		} catch (System.Exception e) {
			print("Exception\n" + jsonStr + "\n" + e.Message);
			if (null != mExceptionHandler) {
				mExceptionHandler("Exception\n" + jsonStr + "\n" + e.Message);
			}
		}
	}
}
