using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Scale : Action {
		public WayType wayType = WayType.To;
		public Vector3 scale = Vector3.zero;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			switch (wayType) {
			case WayType.From:
				tweeners.Add(transform.DOScale(scale, duration).From());
				break;
			case WayType.To:
				tweeners.Add(transform.DOScale(scale, duration));
				break;
			}
			return tweeners;
		}
	}
}
