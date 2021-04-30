using System.Collections;

public static partial class JSONHelper {
	public static JSONObject FromJson(string str) {
		return JSONObject.Create(str);
	}

	public static string ToJson(JSONObject obj) {
		return obj.ToString();
	}
}
