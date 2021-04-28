using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Value : Action {
		public WayType wayType = WayType.To;
		public float from = 0.0f;
		public float to = 1.0f;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			switch (wayType) {
			case WayType.From:
				tweeners.Add(DOVirtual.Float(from, to, duration, onVirtualUpdate).From());
				break;
			case WayType.To:
				tweeners.Add(DOVirtual.Float(from, to, duration, onVirtualUpdate));
				break;
			}
			return tweeners;
		}

		protected override void bindUpdate(Tweener tweener) {
		}

		void onVirtualUpdate(float value) {
			if (null != _updateHandler) {
				_updateHandler(value);
			}
		}
	}
}
