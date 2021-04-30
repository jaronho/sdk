using UnityEngine;
using System.Collections;

namespace iTweenAction {
	public class Action : MonoBehaviour {
		public iTween.EaseType easeType = iTween.EaseType.linear;	// 缓动类型
		public iTween.LoopType loopType = iTween.LoopType.none;		// 循环类型
		public float delay = 0.0f;									// 延迟时间
		public float time = 1.0f;									// 执行时间
		public bool autoDetach = true;								// 是否从物件自动分离(当loopType=none时有效)
		public bool autoDestroy = false;							// 是否自动销毁物件(当loopType=none时有效)

		public delegate void Handler(GameObject obj);

		protected string ID;
		protected Handler _startHandler;
		protected Handler _endHandler;
		protected iTween.EaseType _easeType;
		protected iTween.LoopType _loopType;

		// Use this for initialization
		void Start() {
			ID = System.Guid.NewGuid().ToString();
			_easeType = easeType;
			_loopType = loopType;
			restart();
		}

		// Update is called once per frame
		void Update() {
			if (easeType != _easeType || loopType != _loopType) {
				_easeType = easeType;
				_loopType = loopType;
				iTween comp = iTween.getById(gameObject, ID);
				if (null != comp) {
					comp.easeType = _easeType;
					comp.loopType = _loopType;
				}
			}
		}

		void onStart(string id) {
			if (ID == id) {
				if (null != _startHandler) {
					_startHandler(gameObject);
				}
			}
		}

		void onEnd(string id) {
			if (ID == id) {
				if (iTween.LoopType.none == loopType) {
					if (autoDetach) {
						Destroy(this);
					}
					if (autoDestroy) {
						Destroy(gameObject);
					}
				}
				if (null != _endHandler) {
					_endHandler(gameObject);
				}
			}
		}

		// 重新开始
		public virtual void restart() {
		}

		// 获取id
		public string getId() {
			return ID;
		}

		// 设置开始回调
		public void setStartHandler(Handler handler) {
			_startHandler = handler;
		}

		// 设置结束回调
		public void setEndHandler(Handler handler) {
			_endHandler = handler;
		}

		// 停止
		public void stop() {
			iTween.StopByName(gameObject, ID);
			if (autoDetach) {
				Destroy(this);
			}
			if (autoDestroy) {
				Destroy(gameObject);
			}
		}
	}
}
