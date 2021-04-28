using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class PathGizmos : MonoBehaviour {
		public bool isDrawPath = true;
		public PathType pathType = PathType.CatmullRom;
		public Color gizmosColor = Color.red;
		public int segment = 10;

		#if UNITY_EDITOR
		void OnDrawGizmos() {
			segment = segment > 1 ? segment : 1;
			if (isDrawPath && transform.childCount > 1) {
				Transform beginTransform = null;
				Vector3[] waypoints = new Vector3[transform.childCount];
				for (int i = 0, len = transform.childCount; i < len; ++i) {
					Transform child = transform.GetChild(i);
					waypoints[i] = child.localPosition;
					if (0 == i) {
						beginTransform = child;
					}
				}
				Tweener tweener = beginTransform.DOLocalPath(waypoints, 0, pathType);
				tweener.Pause().ForceInit();
				Vector3[] drawPoints = tweener.PathGetDrawPoints(segment);
				tweener.Kill();
				Gizmos.color = gizmosColor;
				Vector3 prevPt = drawPoints[0];
				for (int i = 1, len = drawPoints.Length; i < len; ++i) {
					Vector3 currPt = drawPoints[i];
					Gizmos.DrawLine(currPt, prevPt);
					prevPt = currPt;
				}
			}
		}
		#endif
	}
}
