using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class LightFrom : Action {
		public float intensity = 0.0f;				// 灯照强度

		void onUpdate(float value) {
			Light l = GetComponent<Light>();
			if (null != l) {
				l.intensity = value;
			}
		}

		// 重新开始
		public override void restart() {
			Light l = GetComponent<Light>();
			if (null == l) {
				Destroy(this);
				return;
			}
			Hashtable args = new Hashtable();
			// 设置id
			args.Add("id", ID);
			// 设置名称
			args.Add("name", ID);
			// 设置类型为线性,线性效果会好一些
			args.Add("easeType", easeType);
			// 三个循环类型:none,loop,pingPong(一次,循环,来回)
			args.Add("loopType", loopType);
			// 设置开始值
			args.Add("from", intensity);
			// 设置结束值
			args.Add("to", l.intensity);
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置缩放时间
			args.Add("time", time);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画更新时调用
			args.Add("onupdate", "onUpdate");
			args.Add("onupdatetarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让值开始变化
			iTween.ValueTo(gameObject, args);
		}
	}
}
