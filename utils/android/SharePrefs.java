package com.jaronho.sdk.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.v4.content.SharedPreferencesCompat;

import java.util.Map;

/**
 * Author:  jaron.ho
 * Date:    2017-05-22
 * Brief:   偏好数据存储
 */

public class SharePrefs {
    private SharedPreferences mSharedPreferences = null;
    private SharedPreferences.Editor mEditor = null;
    private SharedPreferencesCompat.EditorCompat mEditorCompat = null;
    private boolean mAutoSave = true;
    private String mFileName = "";

    /**
     * 功  能: 构造函数
     * 参  数: context - 上下文
     *         name - 文件名
     *         mode - 文件读写模式
     *         autoSave - 是否自动保存
     * 返回值: 无
     */
    public SharePrefs(Context context, String name, int mode, boolean autoSave) {
        mSharedPreferences = context.getSharedPreferences(name, mode);
        mEditor = mSharedPreferences.edit();
        mEditorCompat = SharedPreferencesCompat.EditorCompat.getInstance();
        mEditorCompat.apply(mEditor);
        mAutoSave = autoSave;
        mFileName = name;
    }

    /**
     * 功  能: 构造函数,文件读写模式默认为私有
     * 参  数: context - 上下文
     *         name - 文件名
     *         autoSave - 是否自动保存
     * 返回值: 无
     */
    public SharePrefs(Context context, String name, boolean autoSave) {
        this(context, name, Context.MODE_PRIVATE, autoSave);
    }

    /**
     * 功  能: 构造函数,文件读写模式默认为私有,默认自动保存
     * 参  数: context - 上下文
     *         name - 文件名
     * 返回值: 无
     */
    public SharePrefs(Context context, String name) {
        this(context, name, true);
    }

    /**
     * 功  能: 获取文件名
     * 参  数: 无
     * 返回值: String
     */
    public String getFileName() {
        return mFileName;
    }

    /**
     * 功  能: 是否包含键值
     * 参  数: key - 键值
     * 返回值: boolean
     */
    public boolean contains(String key) {
        return mSharedPreferences.contains(key);
    }

    /**
     * 功  能: 获取整型
     * 参  数: key - 键值
     *         defValue - 默认值
     * 返回值: int
     */
    public int getInt(String key, int defValue) {
        return mSharedPreferences.getInt(key, defValue);
    }

    /**
     * 功  能: 获取整型,默认值为0
     * 参  数: key - 键值
     * 返回值: int
     */
    public int getInt(String key) {
        return getInt(key, 0);
    }

    /**
     * 功  能: 获取长整型
     * 参  数: key - 键值
     *         defValue - 默认值
     * 返回值: long
     */
    public long getLong(String key, long defValue) {
        return mSharedPreferences.getLong(key, defValue);
    }

    /**
     * 功  能: 获取长整型,默认值为0
     * 参  数: key - 键值
     * 返回值: long
     */
    public long getLong(String key) {
        return getLong(key, 0);
    }

    /**
     * 功  能: 获取浮点型
     * 参  数: key - 键值
     *         defValue - 默认值
     * 返回值: float
     */
    public float getFloat(String key, float defValue) {
        return mSharedPreferences.getFloat(key, defValue);
    }

    /**
     * 功  能: 获取浮点型,默认值为0
     * 参  数: key - 键值
     * 返回值: float
     */
    public float getFloat(String key) {
        return getFloat(key, 0);
    }

    /**
     * 功  能: 获取布尔型
     * 参  数: key - 键值
     *         defValue - 默认值
     * 返回值: boolean
     */
    public boolean getBoolean(String key, boolean defValue) {
        return mSharedPreferences.getBoolean(key, defValue);
    }

    /**
     * 功  能: 获取布尔型,默认值为false
     * 参  数: key - 键值
     * 返回值: boolean
     */
    public boolean getBoolean(String key) {
        return getBoolean(key, false);
    }

    /**
     * 功  能: 获取字符串
     * 参  数: key - 键值
     *         defValue - 默认值
     * 返回值: String
     */
    public String getString(String key, String defValue) {
        return mSharedPreferences.getString(key, defValue);
    }

    /**
     * 功  能: 获取字符串,默认值为""
     * 参  数: key - 键值
     * 返回值: String
     */
    public String getString(String key) {
        return getString(key, "");
    }

    /**
     * 功  能: 获取所有值
     * 参  数: 无
     * 返回值: Map<String, ?>
     */
    public Map<String, ?> getAll() {
        return mSharedPreferences.getAll();
    }

    /**
     * 功  能: 设置整型
     * 参  数: key - 键值
     *         value - 值
     * 返回值: 无
     */
    public void setInt(String key, int value) {
        mEditor.putInt(key, value);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 设置长整型
     * 参  数: key - 键值
     *         value - 值
     * 返回值: 无
     */
    public void setLong(String key, long value) {
        mEditor.putLong(key, value);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 设置浮点型
     * 参  数: key - 键值
     *         value - 值
     * 返回值: 无
     */
    public void setFloat(String key, float value) {
        mEditor.putFloat(key, value);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 设置布尔型
     * 参  数: key - 键值
     *         value - 值
     * 返回值: 无
     */
    public void setBoolean(String key, boolean value) {
        mEditor.putBoolean(key, value);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 设置字符串
     * 参  数: key - 键值
     *         value - 值
     * 返回值: 无
     */
    public void setString(String key, String value) {
        mEditor.putString(key, value);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 移除键值
     * 参  数: key - 键值
     * 返回值: 无
     */
    public void remove(String key) {
        mEditor.remove(key);
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 清空所有值
     * 参  数: 无
     * 返回值: 无
     */
    public void clear() {
        mEditor.clear();
        if (mAutoSave) {
            mEditorCompat.apply(mEditor);
        }
    }

    /**
     * 功  能: 手动保存
     * 参  数: 无
     * 返回值: 无
     */
    public void save() {
        mEditorCompat.apply(mEditor);
    }
}
