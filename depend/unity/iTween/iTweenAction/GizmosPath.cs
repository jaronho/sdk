using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace iTweenAction {
	public class GizmosPath : MonoBehaviour {
		public bool isDrawPath = true;				// 是否绘制路劲
		public Color lineColor = Color.yellow;		// 线颜色
		public Color pathColor = Color.red;			// 路径颜色
		
		void OnDrawGizmos() {
			if (isDrawPath && transform.childCount > 0) {
				Transform[] path = new Transform[transform.childCount];
				int index = 0;
				foreach (Transform child in transform) {
					path[index++] = child;
				}
				// 在scene视图中绘制出线/路径
				iTween.DrawLine(path, lineColor);
				iTween.DrawPath(path, pathColor);
			}
		}
	}
}
