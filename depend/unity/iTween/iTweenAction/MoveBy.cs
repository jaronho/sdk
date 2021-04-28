using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class MoveBy : Action {
		public Vector3 amount = Vector3.zero;						// 移动位移
		public bool isLocal = true;									// 是否相对本地(true:相对于自身,false:相对于世界)
		public bool isOrientToPath = true;							// 是否朝向路径

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
			// 设置移动位移
			args.Add("amount", amount);
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置移动时间
			args.Add("time", time);
			// 是否使用局部坐标系(默认为false)
			args.Add("islocal", isLocal);
			// 是否让模型始终面朝当面目标的方向,拐弯的地方会自动旋转模型
			// 如果你发现你的模型在寻路的时候始终都是一个方向那么一定要打开这个
			args.Add("orienttopath", isOrientToPath);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让模型开始寻路
			iTween.MoveBy(gameObject, args);
		}
	}
}
