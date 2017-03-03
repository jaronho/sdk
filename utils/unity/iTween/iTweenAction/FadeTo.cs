using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class FadeTo : Action {
		public float alpha = 0.0f;									// 透明度
		public bool isIncludeChildren = true;						// 是否包括子对象

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
			// 最终透明度
			args.Add("alpha", alpha);
			// 延迟执行时间
			args.Add("delay", delay);
			// 设置渐变时间
			args.Add("time", time);
			// 是否包括子对象
			args.Add("includechildren", isIncludeChildren);
			// 当效果是应用在renderer(渲染器)组件上时,此参数确定具体应用到那个以命名颜色值上
			args.Add("namedcolorvalue", iTween.NamedValueColor._Color);
			// 动画开始时调用
			args.Add("onstart", "onStart");
			args.Add("onstartparams", ID);
			args.Add("onstarttarget", gameObject);
			// 动画结束时调用
			args.Add("oncomplete", "onEnd");
			args.Add("oncompleteparams", ID);
			args.Add("oncompletetarget", gameObject);
			// 让模型开始渐变
			iTween.FadeTo(gameObject, args);
		}
	}
}
