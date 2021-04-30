using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Move : Action {
		public WayType wayType = WayType.To;
		public bool snapping = false;
		public Vector3 position = Vector3.zero;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			switch (wayType) {
			case WayType.From:
				tweeners.Add(transform.DOLocalMove(position, duration, snapping).From());
				break;
			case WayType.To:
				tweeners.Add(transform.DOLocalMove(position, duration, snapping));
				break;
			}
			return tweeners;
		}
	}
}
