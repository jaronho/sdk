using UnityEngine;
using UnityEngine.SceneManagement;
using System.Collections;

public class SceneLoader : MonoBehaviour {
	public string SceneName = "";		// 场景名
	public bool Smooth = false;			// 是否进度平滑过渡

	public delegate void Handler(int progress);

	private Handler _progressHandler;

	// Use this for initialization
	void Start() {
		if (SceneName.Length > 0) {
			execute(SceneName);
		}
	}

	IEnumerator ISceneLoad(string sceneName) {
		if (0 == sceneName.Length) {
			throw new UnityException("scene name is empty");
		}
		int realProgress = 0;
		int displayProgress = 0;
		AsyncOperation op = SceneManager.LoadSceneAsync(sceneName);
		op.allowSceneActivation = false;
		while (op.progress < 0.9f) {	// 这里不用op.isDone,是由于progress增加到0.9f就不变了
			realProgress = (int)op.progress * 100;
			if (Smooth) {
				while (displayProgress < realProgress) {
					++displayProgress;
					if (null != _progressHandler) {
						_progressHandler(displayProgress);
					}
					yield return new WaitForEndOfFrame();
				}
			} else {
				if (null != _progressHandler) {
					_progressHandler(realProgress);
				}
			}
		}
		realProgress = 100;
		if (Smooth) {
			while (displayProgress < realProgress) {
				++displayProgress;
				if (null != _progressHandler) {
					_progressHandler(displayProgress);
				}
				yield return new WaitForEndOfFrame();
			}
		} else {
			if (null != _progressHandler) {
				_progressHandler(realProgress);
			}
		}
		op.allowSceneActivation = true;
	}

	// 执行加载
	public void execute(string sceneName = "") {
		StartCoroutine(ISceneLoad(sceneName));
	}

	// 设置进度回调
	public void setProgressHandler(Handler handler) {
		_progressHandler = handler;
	}
}
