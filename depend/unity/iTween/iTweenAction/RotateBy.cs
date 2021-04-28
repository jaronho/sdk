using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class RotateBy : Action {
		public Vector3 amount = new Vector3(0, 0, 0);				// 旋转位移
		public bool isLocal = true;									// 是否相对本地(true:相对于自身,false:相对于世界)

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
			// 旋转的角度
			args.Add("amount", amount);
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置旋转时间
			args.Add("time", time);
			// 是否使用局部坐标系(默认为false)
			args.Add("islocal", isLocal);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让模型开始旋转
			iTween.RotateBy(gameObject, args);
		}
	}
}
