package com.yaxon.hudmain.jh.library;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.UUID;

public abstract class JHListener {
	private String mId = "";
	private HashMap<String, Object> mObjectMap = new HashMap<>();
	
	public JHListener() {
        mId = UUID.randomUUID().toString();
    }
	
	public JHListener(String id) {
		mId = id;
	}
	
	public String getId() {
		return mId;
	}
	
	public Object get(String key, Object def) {
		if (mObjectMap.containsKey(key)) {
			return mObjectMap.get(key);
		}
		return def;
	}

	public Object getObject(String key) {
		return get(key, null);
	}

	public int getInt(String key) {
		return (int)get(key, 0);
	}

	public long getLong(String key) {
		return (long)get(key, 0);
	}

	public float getFloat(String key) {
		return (float)get(key, 0.0f);
	}

	public double getDouble(String key) {
		return (double)get(key, 0.0f);
	}

	public String getString(String key) {
		return (String)get(key, "");
	}

    public JSONObject getJSONObject(String key) {
        return (JSONObject)get(key, JSONObject.NULL);
    }

	public void set(String key, Object o) {
		mObjectMap.put(key, o);
	}
	
	public abstract void onCallback(int what, Object param);
}
