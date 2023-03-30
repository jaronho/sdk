#pragma once
#include "inifile.h"

namespace ini
{
/**
 * @brief INI文件查看器
 */
class IniReader : protected IniFile
{
public:
    virtual ~IniReader() = default;

    /**
     * @brief 打开文件
     * @param filename 文件名, 例如: "test.ini", "../test.ini", "temp\\test.ini"
     * @param errorDesc [输出]错误信息
     * @return 0-成功, 1-文件无法打开, 2-不匹配']', 3-节名称为空, 4-节名称重复, 5-项解析出错, 6-键名称重复
     */
    int open(const std::string& filename, std::string& errorDesc);

    /**
     * @brief 重载文件
     * @param errorDesc [输出]错误信息
     * @return 0-成功, 1-文件无法打开, 2-不匹配']', 3-节名称为空, 4-节名称重复, 5-项解析出错, 6-键名称重复
     */
    int reload(std::string& errorDesc);

    /**
     * @brief 获取节列表
     * @return 节列表
     */
    std::vector<IniSection> getSections() const;

    /**
     * @brief 获取注释标识符列表
     * @return 标识符列表
     */
    std::vector<std::string> getCommentFlags() const;

    /**
     * @brief 是否存在节
     * @param name 节名称
     * @return true-存在, false-不存在
     */
    bool hasSection(const std::string& name) const;

    /**
     * @brief 获取节注释
     * @param name 节名称
     * @param comment [输出]注释
     * @return true-成功, flalse-失败(不存在)
     */
    bool getSectionComment(const std::string& name, std::string& comment) const;

    /**
     * @brief 是否存在项
     * @param name 节名称
     * @param key 键
     * @return true-存在, false-不存在
     */
    bool hasItem(const std::string& name, const std::string& key) const;

    /**
     * @brief 获取项注释
     * @param name 节名称
     * @param key 键
     * @param comment [输出]注释
     * @return true-成功, flalse-失败(不存在)
     */
    bool getComment(const std::string& name, const std::string& key, std::string& comment) const;

    /**
     * @brief 获取布尔型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    bool getBool(const std::string& name, const std::string& key, bool defaultValue = false) const;

    /**
     * @brief 获取整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    int getInt(const std::string& name, const std::string& key, int defaultValue = 0) const;

    /**
     * @brief 获取无符号整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    unsigned int getUInt(const std::string& name, const std::string& key, unsigned int defaultValue = 0) const;

    /**
     * @brief 获取长整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    long getLong(const std::string& name, const std::string& key, long defaultValue = 0) const;

    /**
     * @brief 获取无符号长整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    unsigned long getULong(const std::string& name, const std::string& key, unsigned long defaultValue = 0) const;

    /**
     * @brief 获取64位长整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    long long getLongLong(const std::string& name, const std::string& key, long long defaultValue = 0) const;

    /**
     * @brief 获取无符号64位长整型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    unsigned long long getULongLong(const std::string& name, const std::string& key, unsigned long long defaultValue = 0) const;

    /**
     * @brief 获取单精度浮点型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    float getFloat(const std::string& name, const std::string& key, float defaultValue = 0.0f) const;

    /**
     * @brief 获取双精度浮点型
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    double getDouble(const std::string& name, const std::string& key, double defaultValue = 0.0) const;

    /**
     * @brief 获取字符串
     * @param name 节名称
     * @param key 键
     * @param defaultValue 默认值
     * @return 值
     */
    std::string getString(const std::string& name, const std::string& key, std::string defaultValue = std::string()) const;

    /**
     * @brief 是否允许创建节/项
     * @return false-不允许
     */
    bool isAllowAutoCreate() const override;

private:
    std::string m_filename; /* 配置文件名 */
};
} // namespace ini
