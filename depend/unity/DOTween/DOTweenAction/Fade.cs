using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class Fade : Action {
		public WayType wayType = WayType.To;
		public float alpha = 0.0f;
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
			Renderer renderer = obj.GetComponent<Renderer>();
			SpriteRenderer spriteRenderer = obj.GetComponent<SpriteRenderer>();
			Image image = obj.GetComponent<Image>();
			Text text = obj.GetComponent<Text>();
			switch (wayType) {
			case WayType.From:
				if (null != renderer) {
					for (int i = 0, len = renderer.materials.Length; i < len; ++i) {
						if (null != renderer.materials[i]) {
							tweeners.Add(renderer.materials[i].DOFade(alpha, duration).From());
						}
					}
				}
				if (null != spriteRenderer) {
					tweeners.Add(spriteRenderer.DOFade(alpha, duration).From());
				}
				if (null != image) {
					tweeners.Add(image.DOFade(alpha, duration).From());
				}
				if (null != text) {
					tweeners.Add(text.DOFade(alpha, duration).From());
				}
				break;
			case WayType.To:
				if (null != renderer) {
					for (int i = 0, len = renderer.materials.Length; i < len; ++i) {
						if (null != renderer.materials[i]) {
							tweeners.Add(renderer.materials[i].DOFade(alpha, duration));
						}
					}
				}
				if (null != spriteRenderer) {
					tweeners.Add(spriteRenderer.DOFade(alpha, duration));
				}
				if (null != image) {
					tweeners.Add(image.DOFade(alpha, duration));
				}
				if (null != text) {
					tweeners.Add(text.DOFade(alpha, duration));
				}
				break;
			}
		}
	}
}
