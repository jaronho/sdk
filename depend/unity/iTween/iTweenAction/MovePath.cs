using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class MovePath : Action {
		public Transform[] path;									// 移动路径
		public bool isMoveToPath = false;							// 是否移动到路径
		public bool isOrientToPath = true;							// 是否朝向路径
		public bool isLocal = true;									// 是否相对本地(true:相对于自身,false:相对于世界)
		public bool isDrawPath = false;								// 是否绘制路径

		void OnDrawGizmos() {
			if (isDrawPath && path.Length > 0) {
				// 在scene视图中绘制出路径与线
				iTween.DrawLine(path, Color.yellow);
				iTween.DrawPath(path, Color.red);
			}
		}

		// 重新开始
		public override void restart() {
			if (path.Length > 0) {
				Hashtable args = new Hashtable();
				// 设置id
				args.Add("id", ID);
				// 设置名称
				args.Add("name", ID);
				// 设置类型为线性,线性效果会好一些
				args.Add("easeType", easeType);
				// 三个循环类型:none,loop,pingPong(一次,循环,来回)
				args.Add("loopType", loopType);
				// 设置移动路径
				args.Add("path", path);
				// 延迟执行时间
				args.Add("delay", delay);
				// 设置移动时间
				args.Add("time", time);
				// 是否先从原始位置走到路径中第一个点的位置
				args.Add("movetopath", isMoveToPath);
				// 是否让模型始终面朝当面目标的方向,拐弯的地方会自动旋转模型
				// 如果你发现你的模型在寻路的时候始终都是一个方向那么一定要打开这个
				args.Add("orienttopath", isOrientToPath);
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
				// 让模型开始寻路
				iTween.MoveTo(gameObject, args);
			}
		}
	}
}
