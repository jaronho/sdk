using UnityEngine;

public static partial class JSONHelper {
	/*
	 * Vector2
	 */
	public static string FromVector2(Vector2 v) {
		JSONObject vdata = JSONObject.obj;
		vdata.AddField("x", v.x);
		vdata.AddField("y", v.y);
		return vdata.ToString();
	}
	public static Vector2 ToVector2(string str) {
		JSONObject obj = JSONObject.Create(str);
		float x = obj["x"] ? obj["x"].f : 0;
		float y = obj["y"] ? obj["y"].f : 0;
		return new Vector2(x, y);
	}

	/*
	 * Vector3
	 */
	public static string FromVector3(Vector3 v) {
		JSONObject vdata = JSONObject.obj;
		vdata.AddField("x", v.x);
		vdata.AddField("y", v.y);
		vdata.AddField("z", v.z);
		return vdata.ToString();
	}
	public static Vector3 ToVector3(string str) {
		JSONObject obj = JSONObject.Create(str);
		float x = obj["x"] ? obj["x"].f : 0;
		float y = obj["y"] ? obj["y"].f : 0;
		float z = obj["z"] ? obj["z"].f : 0;
		return new Vector3(x, y, z);
	}

	/*
	 * Vector4
	 */
	public static string FromVector4(Vector4 v) {
		JSONObject vdata = JSONObject.obj;
		vdata.AddField("x", v.x);
		vdata.AddField("y", v.y);
		vdata.AddField("z", v.z);
		vdata.AddField("w", v.w);
		return vdata.ToString();
	}
	public static Vector4 ToVector4(string str) {
		JSONObject obj = JSONObject.Create(str);
		float x = obj["x"] ? obj["x"].f : 0;
		float y = obj["y"] ? obj["y"].f : 0;
		float z = obj["z"] ? obj["z"].f : 0;
		float w = obj["w"] ? obj["w"].f : 0;
		return new Vector4(x, y, z, w);
	}

	/*
	 * Matrix4x4
	 */
	public static string FromMatrix4x4(Matrix4x4 m) {
		JSONObject mdata = JSONObject.obj;
		mdata.AddField("m00", m.m00);
		mdata.AddField("m01", m.m01);
		mdata.AddField("m02", m.m02);
		mdata.AddField("m03", m.m03);
		mdata.AddField("m10", m.m10);
		mdata.AddField("m11", m.m11);
		mdata.AddField("m12", m.m12);
		mdata.AddField("m13", m.m13);
		mdata.AddField("m20", m.m20);
		mdata.AddField("m21", m.m21);
		mdata.AddField("m22", m.m22);
		mdata.AddField("m23", m.m23);
		mdata.AddField("m30", m.m30);
		mdata.AddField("m31", m.m31);
		mdata.AddField("m32", m.m32);
		mdata.AddField("m33", m.m33);
		return mdata.ToString();
	}
	public static Matrix4x4 ToMatrix4x4(string str) {
		JSONObject obj = JSONObject.Create(str);
		Matrix4x4 result = new Matrix4x4();
		if(obj["m00"]) result.m00 = obj["m00"].f;
		if(obj["m01"]) result.m01 = obj["m01"].f;
		if(obj["m02"]) result.m02 = obj["m02"].f;
		if(obj["m03"]) result.m03 = obj["m03"].f;
		if(obj["m10"]) result.m10 = obj["m10"].f;
		if(obj["m11"]) result.m11 = obj["m11"].f;
		if(obj["m12"]) result.m12 = obj["m12"].f;
		if(obj["m13"]) result.m13 = obj["m13"].f;
		if(obj["m20"]) result.m20 = obj["m20"].f;
		if(obj["m21"]) result.m21 = obj["m21"].f;
		if(obj["m22"]) result.m22 = obj["m22"].f;
		if(obj["m23"]) result.m23 = obj["m23"].f;
		if(obj["m30"]) result.m30 = obj["m30"].f;
		if(obj["m31"]) result.m31 = obj["m31"].f;
		if(obj["m32"]) result.m32 = obj["m32"].f;
		if(obj["m33"]) result.m33 = obj["m33"].f;
		return result;
	}
	/*
	 * Quaternion
	 */
	public static string FromQuaternion(Quaternion q) {
		JSONObject qdata = JSONObject.obj;
		qdata.AddField("w", q.w);
		qdata.AddField("x", q.x);
		qdata.AddField("y", q.y);
		qdata.AddField("z", q.z);
		return qdata.ToString();
	}
	public static Quaternion ToQuaternion(string str) {
		JSONObject obj = JSONObject.Create(str);
		float x = obj["x"] ? obj["x"].f : 0;
		float y = obj["y"] ? obj["y"].f : 0;
		float z = obj["z"] ? obj["z"].f : 0;
		float w = obj["w"] ? obj["w"].f : 0;
		return new Quaternion(x, y, z, w);
	}

	/*
	 * Color
	 */
	public static string FromColor(Color c) {
		JSONObject cdata = JSONObject.obj;
		cdata.AddField("r", c.r);
		cdata.AddField("g", c.g);
		cdata.AddField("b", c.b);
		cdata.AddField("a", c.a);
		return cdata.ToString();
	}
	public static Color ToColor(string str) {
		JSONObject obj = JSONObject.Create(str);
		Color c = new Color();
		for (int i = 0; i < obj.Count; ++i) {
			switch (obj.keys[i]) {
			case "r": c.r = obj[i].f; break;
			case "g": c.g = obj[i].f; break;
			case "b": c.b = obj[i].f; break;
			case "a": c.a = obj[i].f; break;
			}
		}
		return c;
	}

	/*
	 * Layer Mask
	 */
	public static string FromLayerMask(LayerMask l) {
		JSONObject result = JSONObject.obj;
		result.AddField("value", l.value);
		return result.ToString();
	}
	public static LayerMask ToLayerMask(string str) {
		JSONObject obj = JSONObject.Create(str);
		LayerMask l = new LayerMask {value = (int)obj["value"].n};
		return l;
	}

	/*
	 * Rect
	 */
	public static string FromRect(Rect r) {
		JSONObject result = JSONObject.obj;
		result.AddField("x", r.x);
		result.AddField("y", r.y);
		result.AddField("height", r.height);
		result.AddField("width", r.width);
		return result.ToString();
	}
	public static Rect ToRect(string str) {
		JSONObject obj = JSONObject.Create(str);
		Rect r = new Rect();
		for(int i = 0; i < obj.Count; ++i) {
			switch (obj.keys[i]) {
			case "x": r.x = obj[i].f; break;
			case "y": r.y = obj[i].f; break;
			case "height": r.height = obj[i].f; break;
			case "width": r.width = obj[i].f; break;
			}
		}
		return r;
	}

	/*
	 * Rect Offset
	 */
	public static string FromRectOffset(RectOffset r) {
		JSONObject result = JSONObject.obj;
		result.AddField("bottom", r.bottom);
		result.AddField("left", r.left);
		result.AddField("right", r.right);
		result.AddField("top", r.top);
		return result.ToString();
	}
	public static RectOffset ToRectOffset(string str) {
		JSONObject obj = JSONObject.Create(str);
		RectOffset r = new RectOffset();
		for (int i = 0; i < obj.Count; ++i) {
			switch (obj.keys[i]) {
			case "bottom": r.bottom = (int)obj[i].n; break;
			case "left": r.left = (int)obj[i].n; break;
			case "right": r.right =	(int)obj[i].n; break;
			case "top": r.top = (int)obj[i].n; break;
			}
		}
		return r;
	}

	/*
	 * Key frame
	 */
	static JSONObject FromKeyFrameImpl(Keyframe k) {
		JSONObject result = JSONObject.obj;
		result.AddField("inTangent", k.inTangent);
		result.AddField("outTangent", k.outTangent);
		result.AddField("tangentMode", k.tangentMode);
		result.AddField("time", k.time);
		result.AddField("value", k.value);
		return result;
	}
	static Keyframe ToKeyFrameImpl(JSONObject obj) {
		Keyframe k = new Keyframe(obj.HasField("time") ? obj.GetField("time").n : 0, obj.HasField("value") ? obj.GetField("value").n : 0);
		if (obj.HasField("inTangent")) {
			k.inTangent = obj.GetField("inTangent").n;
		}
		if (obj.HasField("outTangent")) {
			k.outTangent = obj.GetField("outTangent").n;
		}
		if (obj.HasField("tangentMode")) {
			k.tangentMode = (int)obj.GetField("tangentMode").n;
		}
		return k;
	}
	public static string FromKeyframe(Keyframe k) {
		return FromKeyFrameImpl(k).ToString();
	}
	public static Keyframe ToKeyFrame(string str) {
		return ToKeyFrameImpl(JSONObject.Create(str));
	}

	/*
	 * Animation Curve
	 */
	public static string FromAnimationCurve(AnimationCurve a) {
		JSONObject result = JSONObject.obj;
		result.AddField("preWrapMode", a.preWrapMode.ToString()); 
		result.AddField("postWrapMode", a.postWrapMode.ToString()); 
		if (a.keys.Length > 0) {
			JSONObject keysJSON = JSONObject.Create();
			for (int i = 0; i < a.keys.Length; ++i) {
				keysJSON.Add(FromKeyFrameImpl(a.keys[i]));
			}
			result.AddField("keys", keysJSON);
		}
		return result.ToString();
	}
	public static AnimationCurve ToAnimationCurve(string str) {
		JSONObject obj = JSONObject.Create(str);
		AnimationCurve a = new AnimationCurve();
		if (obj.HasField("keys")) {
			JSONObject keys = obj.GetField("keys");
			for (int i = 0; i < keys.list.Count; ++i) {
				a.AddKey(ToKeyFrameImpl(keys[i]));
			}
		}
		if (obj.HasField("preWrapMode")) {
			a.preWrapMode = (WrapMode)((int)obj.GetField("preWrapMode").n);
		}
		if (obj.HasField("postWrapMode")) {
			a.postWrapMode = (WrapMode)((int)obj.GetField("postWrapMode").n);
		}
		return a;
	}

	/*
	 * WWW Form
	 */
	public static WWWForm ToWWWForm(string str) {
		JSONObject obj = JSONObject.Create(str);
		WWWForm form = new WWWForm();
		for (int i = 0; i < obj.list.Count; ++i) {
			string key = i + "";
			if (JSONObject.Type.OBJECT == obj.type) {
				key = obj.keys[i];
			}
			string val = obj.list[i].ToString();
			if (JSONObject.Type.STRING == obj.list [i].type) {
				val = val.Replace("\"", "");
			}
			form.AddField(key, val);
		}
		return form;
	}
}
