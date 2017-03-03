using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Path : Action {
		public PathType pathType = PathType.CatmullRom;
		public PathMode pathMode = PathMode.Full3D;
		public int resolution = 5;
		public LookType orientation = LookType.None;
		public float lookAhead = 0.01f;
		public Transform lookTransform;
		public Vector3 lookPosition;
		public Vector3[] waypoints;
		public bool isDrawPath = false;
		public Color gizmosColor = Color.red;
		public int segment = 10;

		public enum LookType {
			None,
			LookAhead,
			LookTransform,
			LookPosition
		}

		protected TweenCallback<int> _waypointChangeHandler;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			if (null != waypoints && waypoints.Length > 0) {
				Vector3[] wps = new Vector3[waypoints.Length];
				for (int i = 0, len = waypoints.Length; i < len; ++i) {
					wps[i] = waypoints[i];
				}
				Tweener tweener = null;
				switch (orientation) {
				case LookType.None:
					tweener = transform.DOLocalPath(wps, duration, pathType, pathMode, resolution);
					break;
				case LookType.LookAhead:
					tweener = transform.DOLocalPath(wps, duration, pathType, pathMode, resolution).SetLookAt(lookAhead);
					break;
				case LookType.LookTransform:
					tweener = transform.DOLocalPath(wps, duration, pathType, pathMode, resolution).SetLookAt(lookTransform);
					break;
				case LookType.LookPosition:
					tweener = transform.DOLocalPath(wps, duration, pathType, pathMode, resolution).SetLookAt(lookPosition);
					break;
				}
				tweener.OnWaypointChange(onTweenerWaypointChange);
				tweeners.Add(tweener);
			}
			return tweeners;
		}

		#if UNITY_EDITOR
		void OnDrawGizmos() {
			segment = segment > 1 ? segment : 1;
			if (isDrawPath && waypoints.Length > 0) {
				Tweener tweener = transform.DOLocalPath(waypoints, 0, pathType);
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

		void onTweenerWaypointChange(int waypointIndex) {
			if (null != _waypointChangeHandler) {
				_waypointChangeHandler(waypointIndex);
			}
		}

		public void setWaypointChangeHandler(TweenCallback<int> handler) {
			_waypointChangeHandler = handler;
		}

		public static Vector3[] getDrawPoints(Transform trans, Vector3[] wps, int seg = 10, PathType type = PathType.CatmullRom) {
			if (null == trans || 0 == wps.Length) {
				return new Vector3[0];
			}
			Vector3 tempPosition = trans.localPosition;
			trans.localPosition = wps[0];
			Tweener tweener = trans.DOLocalPath(wps, 0, type);
			tweener.Pause().ForceInit();
			Vector3[] drawPoints = tweener.PathGetDrawPoints(seg);
			tweener.Kill();
			trans.localPosition = tempPosition;
			return drawPoints;
		}

		public static Vector3[] getPoints(Transform trans, Vector3[] wps, float[] percents, PathType type = PathType.CatmullRom) {
			if (null == trans || 0 == wps.Length) {
				return new Vector3[0];
			}
			Vector3 tempPosition = trans.localPosition;
			trans.localPosition = wps[0];
			Tweener tweener = trans.DOLocalPath(wps, 0, type);
			tweener.Pause().ForceInit();
			Vector3[] points = new Vector3[percents.Length];
			for (int i = 0, len = percents.Length; i < len; ++i) {
				points[i] = tweener.PathGetPoint(percents[i]);
			}
			tweener.Kill();
			trans.localPosition = tempPosition;
			return points;
		}

		public static Vector3 getPoint(Transform trans, Vector3[] wps, float percent, PathType type = PathType.CatmullRom) {
			if (null == trans || 0 == wps.Length) {
				return Vector3.zero;
			}
			Vector3 tempPosition = trans.localPosition;
			trans.localPosition = wps[0];
			Tweener tweener = trans.DOLocalPath(wps, 0, type);
			tweener.Pause().ForceInit();
			Vector3 point = tweener.PathGetPoint(percent);
			tweener.Kill();
			trans.localPosition = tempPosition;
			return point;
		}

		public static float getLength(Transform trans, Vector3[] wps, PathType type = PathType.CatmullRom) {
			if (null == trans || 0 == wps.Length) {
				return 0;
			}
			Vector3 tempPosition = trans.localPosition;
			trans.localPosition = wps[0];
			Tweener tweener = trans.DOLocalPath(wps, 0, type);
			tweener.Pause().ForceInit();
			float length = tweener.PathLength();
			tweener.Kill();
			trans.localPosition = tempPosition;
			return length;
		}
	}
}
