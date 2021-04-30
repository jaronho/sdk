using UnityEngine;
using System.Collections;

public class QualitySetting : MonoBehaviour {
	public int FPS = 30;
	public bool ShowFPS = false;
	public Vector2 OffsetFPS = new Vector2(10, 10);
	public int SizeFPS = 20;
	public Color ColorFPS = Color.red;

	private int _frames = 0;
	private float _lastInterval = 0.0f;
	private GUIText _guiText = null;

	void Awake() {
		QualitySettings.vSyncCount = 0;
		Application.targetFrameRate = FPS;
	}

	void Start() {
		_frames = 0;
		_lastInterval = Time.realtimeSinceStartup;
	}

	void OnDisable() {
		hideFPS();
	}

	void Update() {
		if (ShowFPS) {
			updateFPS(OffsetFPS, SizeFPS, ColorFPS);
		} else {
			hideFPS();
		}
	}

	void updateFPS(Vector2 offset, int fontSize, Color color) {
		++_frames;
		float timeNow = Time.realtimeSinceStartup;
		if (timeNow > _lastInterval + 1.0f) {
			float fps = _frames / (timeNow - _lastInterval);
			float ms = 1000.0f / Mathf.Max(fps, 0.00001f);
			_frames = 0;
			_lastInterval = timeNow;
			if (null == _guiText) {
				GameObject go = new GameObject("FPS Display", typeof(GUIText));
				go.hideFlags = HideFlags.HideAndDontSave;
				_guiText = go.GetComponent<GUIText>();
				_guiText.anchor = TextAnchor.MiddleLeft;
				_guiText.alignment = TextAlignment.Left;
				_guiText.pixelOffset = offset;
				_guiText.fontSize = fontSize;
				_guiText.fontStyle = FontStyle.Bold;
				_guiText.material.color = color;
			}
			_guiText.text = ms.ToString("f1") + "ms " + fps.ToString("f2") + "FPS";
		}
	}

	void hideFPS() {
		_frames = 0;
		_lastInterval = Time.realtimeSinceStartup;
		if (null != _guiText) {
			DestroyImmediate(_guiText.gameObject);
			_guiText = null;
		}
	}
}
