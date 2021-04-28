using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;

namespace DOTWeenAction {
	public enum WayType {
		From,
		To
	}

	public class Action : MonoBehaviour {
		public float timeScale = 1.0f;			// 时间缩放(可实时改变)
		public UpdateType updateType = UpdateType.Normal;
		public bool independentUpdate = false;	// true,忽略Unity的Time.timeScale
		public Ease easeType = Ease.Linear;
		public LoopType loopType = LoopType.Restart;
		public int loopCount = 1;
		public float delay = 0.0f;
		public float duration = 1.0f;
		public float gotoDuration = 0.0f;
		public bool gotoAndPlay = false;
		public bool autoPlay = true;
		public bool autoDestroy = false;

		protected TweenCallback _startHandler;
		protected TweenCallback<float> _updateHandler;
		protected TweenCallback _stepHandler;
		protected TweenCallback _completeHandler;

		private string _id;
		private List<Tweener> _tweeners;
		private bool _played;
		private float _timeScale;

		// Use this for initialization
		void Start() {
			_id = (null == _id || 0 == _id.Length) ? System.Guid.NewGuid().ToString() : _id;
			timeScale = timeScale <= 0 ? 0 : timeScale;
			_timeScale = timeScale;
			if (autoPlay) {
				_played = true;
				_tweeners = immediatePlay();
			}
		}

		// Update is called once per frame
		void Update() {
			timeScale = timeScale <= 0 ? 0 : timeScale;
			if (timeScale != _timeScale) {
				_timeScale = timeScale;
				updateTimeScale();
			}
			if (!_played && autoPlay) {
				_played = true;
				_tweeners = immediatePlay();
			}
		}

		List<Tweener> immediatePlay() {
			List<Tweener> tweeners = play();
			if (null == tweeners || 0 == tweeners.Count) {
				onTweenerKill();
			} else {
				loopCount = loopCount <= 0 ? -1 : loopCount;
				tweeners[0].OnStart(onTweenerStart);
				bindUpdate(tweeners[0]);
				tweeners[0].OnStepComplete(onTweenerStep);
				tweeners[0].OnComplete(onTweenerComplete);
				tweeners[0].OnKill(onTweenerKill);
				for (int i = 0, len = tweeners.Count; i < len; ++i) {
					tweeners[i].timeScale = _timeScale;
					tweeners[i].SetId(_id);
					tweeners[i].SetUpdate(updateType, independentUpdate);
					tweeners[i].SetEase(easeType);
					tweeners[i].SetLoops(loopCount, loopType);
					tweeners[i].SetDelay(delay);
					tweeners[i].SetAutoKill(true);
					tweeners[i].SetRecyclable(true);
				}
				gotoDuration = gotoDuration < 0 ? 0 : (gotoDuration > duration ? duration : gotoDuration);
				if (gotoDuration > 0) {
					for (int i = 0, len = tweeners.Count; i < len; ++i) {
						tweeners[i].Goto(gotoDuration, gotoAndPlay);
					}
				}
			}
			return tweeners;
		}

		void updateTimeScale() {
			if (null != _tweeners && _tweeners.Count > 0) {
				for (int i = 0, len = _tweeners.Count; i < len; ++i) {
					if (null != _tweeners[i]) {
						_tweeners[i].timeScale = _timeScale;
					}
				}
			}
		}

		void onTweenerStart() {
			if (null != _startHandler) {
				_startHandler();
			}
		}

		void onTweenerUpdate() {
			float percent = 0;
			if (null != _tweeners && _tweeners.Count > 0 && null != _tweeners[0]) {
				percent = _tweeners[0].ElapsedPercentage(false);
			}
			if (null != _updateHandler) {
				_updateHandler(percent);
			}
		}

		void onTweenerStep() {
			if (null != _stepHandler) {
				_stepHandler();
			}
		}

		void onTweenerComplete() {
			if (null != _completeHandler) {
				_completeHandler();
			}
		}

		void onTweenerKill() {
			Destroy(this);
			if (autoDestroy) {
				Renderer[] renderers = GetComponentsInChildren<Renderer>(true);
				for (int i = 0, len = renderers.Length; i < len; ++i) {
					for (int j = 0, l = renderers[i].materials.Length; j < l; ++j) {
						Destroy(renderers[i].materials[j]);
					}
				}
				Destroy(gameObject);
			}
		}

		protected virtual List<Tweener> play() {
			return new List<Tweener>();
		}

		protected virtual void bindUpdate(Tweener tweener) {
			if (null != tweener) {
				tweener.OnUpdate(onTweenerUpdate);
			}
		}

		public string ID {
			get {
				return _id;
			}
			set {
				_id = (null == _id || 0 == _id.Length) ? value : _id;
			}
		}

		public void setStartHandler(TweenCallback handler) {
			_startHandler = handler;
		}

		public void setUpdateHandler(TweenCallback<float> handler) {
			_updateHandler = handler;
		}

		public void setStepHandler(TweenCallback handler) {
			_stepHandler = handler;
		}

		public void setCompleteHandler(TweenCallback handler) {
			_completeHandler = handler;
		}

		public void kill() {
			DOTween.Kill(_id);
		}
	}
}
