/**********************************************************************
* Author:	jaron.ho
* Date:		2017-10-18
* Brief:	本地存储
**********************************************************************/
#ifndef _SHARE_PREFS_H_
#define _SHARE_PREFS_H_

#include "xmlhelper/XmlHelper.h"

class SharePrefs {
public:
    /*
     * Brief:	打开
     * Param:	fileName - 文件名, 例如:"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
     *          forceReplace - 强制替换, 当文件已存在但不是xml格式时, 强制删除并重新生成
     * Return:	bool
     */
    static bool open(const std::string& fileName, bool forceReplace = true);

    /*
     * Brief:	保存
     * Param:	void
     * Return:	bool
     */
    static bool save(void);

    /*
     * Brief:	清空
     * Param:	void
     * Return:	bool
     */
    static bool clear(void);

    /*
     * Brief:	获取整型值
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	int
     */
    static int getInt(const std::string& key, int defaultValue = 0);

    /*
     * Brief:	设置整型值
     * Param:	key - 键值
     *			value - 整型值
     * Return:	bool
     */
    static bool setInt(const std::string& key, int value);

    /*
     * Brief:	获取长整型值
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	long
     */
    static long getLong(const std::string& key, long defaultValue = 0);

    /*
     * Brief:	设置长整型值
     * Param:	key - 键值
     *			value - 长整型值
     * Return:	bool
     */
    static bool setLong(const std::string& key, long value);

    /*
     * Brief:	获取单精度值
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	float
     */
    static float getFloat(const std::string& key, float defaultValue = 0.0f);

    /*
     * Brief:	设置单精度值
     * Param:	key - 键值
     *			value - 单精度值
     * Return:	bool
     */
    static bool setFloat(const std::string& key, float value);

    /*
     * Brief:	获取双精度值
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	double
     */
    static double getDouble(const std::string& key, double defaultValue = 0.0);

    /*
     * Brief:	设置双精度值
     * Param:	key - 键值
     *			value - 双精度值
     * Return:	bool
     */
    static bool setDouble(const std::string& key, double value);

    /*
     * Brief:	获取布尔值
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	bool
     */
    static bool getBool(const std::string& key, bool defaultValue = false);

    /*
     * Brief:	设置布尔值
     * Param:	key - 键值
     *			value - 布尔值
     * Return:	bool
     */
    static bool setBool(const std::string& key, bool value);

    /*
     * Brief:	获取字符串
     * Param:	key - 键值
     *			defaultValue - 默认值
     * Return:	string
     */
    static std::string getString(const std::string& key, const std::string& defaultValue = "");

    /*
     * Brief:	设置字符串
     * Param:	key - 键值
     *			value - 字符串
     * Return:	bool
     */
    static bool setString(const std::string& key, const std::string& value);

    /*
     * Brief:	移除
     * Param:	key - 键值
     * Return:	bool
     */
    static bool remove(const std::string& key);
};

#endif // _SHARE_PREFS_H_
