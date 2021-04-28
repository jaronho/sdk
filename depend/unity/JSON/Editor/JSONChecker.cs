//#define PERFTEST        //For testing performance of parse/stringify.  Turn on editor profiling to see how we're doing

using UnityEngine;
using UnityEditor;

public class JSONChecker : EditorWindow {
	string JSON = @"{
		""TestObject"":{
			""SomeText"":""Blah"",
			""SomeObject"":{
				""SomeNumber"":42,
				""SomeFloat"":13.37,
				""SomeBool"":true,
				""SomeNull"":null
			},
			""SomeEmptyObject"":{},
			""SomeEmptyArray"":[],
			""EmbeddedObject"":""{\""field\"":\""Value with \\\""escaped quotes\\\""\""}""
		}
	}";
	string URL = "";
	string error = "";
	JSONObject obj;
	operate_type type;
	enum operate_type {
		none,
		check,
		get
	}

	[MenuItem("Window/JSONChecker")]

	static void Init() {
		GetWindow(typeof(JSONChecker));
	}

	void OnGUI() {
		JSON = EditorGUILayout.TextArea(JSON);
		if (GUILayout.Button("Check JSON")) {
#if PERFTEST
            Profiler.BeginSample("JSONParse");
			obj = JSONObject.Create(JSON);
            Profiler.EndSample();
            Profiler.BeginSample("JSONStringify");
			obj.ToString(true);
            Profiler.EndSample();
#else
			obj = JSONObject.Create(JSON);
#endif
			type = operate_type.check;
		}

		EditorGUILayout.Separator();

		URL = EditorGUILayout.TextField("URL", URL);
		if (GUILayout.Button("Get JSON")) {
			WWW test = new WWW(URL);
			while (!test.isDone);
			if (!string.IsNullOrEmpty(test.error)) {
				obj = null;
				error = test.error;
			} else {
				obj = new JSONObject(test.text);
			}
			type = operate_type.get;
		}

		EditorGUILayout.Separator();

		if (obj) {
			// Debug.Log(System.GC.GetTotalMemory(false) + "");
			if (JSONObject.Type.NULL == obj.type) {
				switch (type) {
				case operate_type.check:
					GUILayout.Label("JSON check fail:\n" + obj.ToString(true));
					break;
				case operate_type.get:
					GUILayout.Label("JSON get [" + URL + "] fail:\n" + error);
					break;
				}
			} else {
				switch (type) {
				case operate_type.check:
					GUILayout.Label("JSON check success:\n" + obj.ToString(true));
					break;
				case operate_type.get:
					GUILayout.Label("JSON get [\" + URL + \"] success:\n" + obj.ToString(true));
					break;
				}
			}
		}
	}
}
