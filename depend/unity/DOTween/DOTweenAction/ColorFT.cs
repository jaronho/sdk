using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public class ColorFT : Action {
		public WayType wayType = WayType.To;
		public Color color = Color.white;
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
			Camera camera = obj.GetComponent<Camera>();
			Light light = obj.GetComponent<Light>();
			Renderer renderer = obj.GetComponent<Renderer>();
			SpriteRenderer spriteRenderer = obj.GetComponent<SpriteRenderer>();
			Image image = obj.GetComponent<Image>();
			Text text = obj.GetComponent<Text>();
			switch (wayType) {
			case WayType.From:
				if (null != camera) {
					tweeners.Add(camera.DOColor(color, duration).From());
				}
				if (null != light) {
					tweeners.Add(light.DOColor(color, duration).From());
				}
				if (null != renderer) {
					for (int i = 0, len = renderer.materials.Length; i < len; ++i) {
						if (null != renderer.materials[i]) {
							tweeners.Add(renderer.materials[i].DOColor(color, duration).From());
						}
					}
				}
				if (null != spriteRenderer) {
					tweeners.Add(spriteRenderer.DOColor(color, duration).From());
				}
				if (null != image) {
					tweeners.Add(image.DOColor(color, duration).From());
				}
				if (null != text) {
					tweeners.Add(text.DOColor(color, duration).From());
				}
				break;
			case WayType.To:
				if (null != camera) {
					tweeners.Add(camera.DOColor(color, duration));
				}
				if (null != light) {
					tweeners.Add(light.DOColor(color, duration));
				}
				if (null != renderer) {
					for (int i = 0, len = renderer.materials.Length; i < len; ++i) {
						if (null != renderer.materials[i]) {
							tweeners.Add(renderer.materials[i].DOColor(color, duration));
						}
					}
				}
				if (null != spriteRenderer) {
					tweeners.Add(spriteRenderer.DOColor(color, duration));
				}
				if (null != image) {
					tweeners.Add(image.DOColor(color, duration));
				}
				if (null != text) {
					tweeners.Add(text.DOColor(color, duration));
				}
				break;
			}
		}
	}
}
