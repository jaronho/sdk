using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Rotate : Action {
		public WayType wayType = WayType.To;
		public Vector3 angle = Vector3.zero;
		public RotateMode rotateMode = RotateMode.Fast;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			switch (wayType) {
			case WayType.From:
				tweeners.Add(transform.DOLocalRotate(angle, duration, rotateMode).From());
				break;
			case WayType.To:
				tweeners.Add(transform.DOLocalRotate(angle, duration, rotateMode));
				break;
			}
			return tweeners;
		}
	}
}
