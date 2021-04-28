using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class ValueFromTo : Action {
		public value_type valueType = value_type.FLOAT;				// 参数类型
		public float floatFrom;										// float开始值
		public float floatTo;										// float结束值
		public Color colorFrom;										// Color开始值
		public Color colorTo;										// Color结束值
		public Vector2 vector2From;									// Vector2开始值
		public Vector2 vector2To;									// Vector2结束值
		public Vector3 vector3From;									// Vector3开始值
		public Vector3 vector3To;									// Vector3结束值
		public Rect rectFrom;										// Rect开始值
		public Rect rectTo;											// Rect结束值

		public enum value_type {
			FLOAT,
			COLOR,
			VECTOR2,
			VECTOR3,
			RECT
		}
		public delegate void UpdateHandler(GameObject obj, System.Object value);

		private UpdateHandler _updateHandler;

		void onUpdateFloat(float value) {
			if (null != _updateHandler) {
				_updateHandler(gameObject, value);
			}
		}

		void onUpdateColor(Color value) {
			if (null != _updateHandler) {
				_updateHandler(gameObject, value);
			}
		}

		void onUpdateVector2(Vector2 value) {
			if (null != _updateHandler) {
				_updateHandler(gameObject, value);
			}
		}

		void onUpdateVector3(Vector3 value) {
			if (null != _updateHandler) {
				_updateHandler(gameObject, value);
			}
		}

		void onUpdateRect(Rect value) {
			if (null != _updateHandler) {
				_updateHandler(gameObject, value);
			}
		}

		// 重新开始
		public override void restart() {
			Hashtable args = new Hashtable();
			// 设置id
			args.Add("id", ID);
			// 设置名称
			args.Add("name", ID);
			// 设置类型为线性,线性效果会好一些
			args.Add("easeType", easeType);
			// 三个循环类型:none,loop,pingPong(一次,循环,来回)
			args.Add("loopType", loopType);
			// 设置开始值/结束值
			switch (valueType) {
			case value_type.FLOAT:
				args.Add("from", floatFrom);
				args.Add("to", floatTo);
				args.Add("onupdate", "onUpdateFloat");
				break;
			case value_type.COLOR:
				args.Add("from", colorFrom);
				args.Add("to", colorTo);
				args.Add("onupdate", "onUpdateColor");
				break;
			case value_type.VECTOR2:
				args.Add("from", vector2From);
				args.Add("to", vector2To);
				args.Add("onupdate", "onUpdateVector2");
				break;
			case value_type.VECTOR3:
				args.Add("from", vector3From);
				args.Add("to", vector3To);
				args.Add("onupdate", "onUpdateVector3");
				break;
			case value_type.RECT:
				args.Add("from", rectFrom);
				args.Add("to", rectTo);
				args.Add("onupdate", "onUpdateRect");
				break;
			}
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置缩放时间
			args.Add("time", time);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画更新时调用
			args.Add("onupdatetarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让值开始变化
			iTween.ValueTo(gameObject, args);
		}

		// 设置更新回调
		public void setUpdateHandler(UpdateHandler handler) {
			_updateHandler = handler;
		}
	}
}
