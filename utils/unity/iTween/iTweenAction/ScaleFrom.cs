using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class ScaleFrom : Action {
		public Vector3 scale = new Vector3(1, 1, 1);				// 缩放参数

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
			// 设置缩放参数
			args.Add("scale", scale);
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置缩放时间
			args.Add("time", time);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让模型开始缩放
			iTween.ScaleFrom(gameObject, args);
		}
	}
}
