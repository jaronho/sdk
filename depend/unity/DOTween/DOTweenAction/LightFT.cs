using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class LightFT : Action {
		public WayType wayType = WayType.To;
		public float intensity = 0.0f;
		public bool includeChildren = true;

		protected override List<Tweener> play() {
			List<Tweener> tweeners = new List<Tweener>();
			playImpl(tweeners, gameObject);
			if (includeChildren) {
				Transform[] children = gameObject.GetComponentsInChildren<Transform>();
				for (int i = 0, len = children.Length; i < len; ++i) {
					if (transform != children[i]) {
						playImpl(tweeners, children[i].gameObject);
					}
				}
			}
			return tweeners;
		}

		void playImpl(List<Tweener> tweeners, GameObject obj) {
			Light light = obj.GetComponent<Light>();
			switch (wayType) {
			case WayType.From:
				if (null != light) {
					tweeners.Add(light.DOIntensity(intensity, duration).From());
				}
				break;
			case WayType.To:
				if (null != light) {
					tweeners.Add(light.DOIntensity(intensity, duration));
				}
				break;
			}
		}
	}
}
