#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ini
{
/**
 * @brief INI项
 */
struct IniItem
{
    std::string key; /* 键 */
    std::string value; /* 值 */
    std::string comment; /* 注释 */
};

/**
 * @brief INI节
 */
struct IniSection
{
    std::string name; /* 节名称 */
    std::vector<IniItem> items; /* 包含的项列表 */
    std::string comment; /* 注释 */
};

/**
 * @brief INI文件
 */
class IniFile
{
public:
    IniFile();

    virtual ~IniFile();

    /**
     * @brief 打开文件
     * @param filename 文件名, 例如: "test.ini", "../test.ini", "temp\\test.ini"
     * @param allowTailComment 是否允许在行后面添加注释
     * @param lineLength 每行的字符个数限制
     * @return 0-成功, 1-文件无法打开, 2-不匹配']', 3-节名称为空, 4-节名称重复, 5-项解析出错
     */
    int open(const std::string& filename, bool allowTailComment = false, size_t lineLength = 128);

    /**
     * @brief 保存文件(对象在析构时会自动调用该接口)
     * @return true-成功, false-失败
     */
    bool save();

    /**
     * @brief 清除
     */
    void clear();

    /**
     * @brief 获取数据列表
     * @return 数据列表
     */
    std::vector<IniSection> getDataList();

    /**
     * @brief 获取注释标识符列表
     * @return 标识符列表
     */
    std::vector<std::string> getCommentFlags();

    /**
     * @brief 设置注释标识符列表
     * @param flags 标识符列表
     * @return true-成功, flalse-失败
     */
    bool setCommentFlags(const std::vector<std::string>& flags);

    /**
     * @brief 是否存在节
     * @param sectionName 节名称
     * @return true-存在, false-不存在
     */
    bool hasSection(const std::string& sectionName);

    /**
     * @brief 删除节
     * @param nameName 节名称
     * @return true-成功, flalse-失败(不存在)
     */
    bool removeSection(const std::string& sectionName);

    /**
     * @brief 获取节注释
     * @param sectionName 节名称
     * @param comment [输出]注释
     * @return true-成功, flalse-失败(不存在)
     */
    bool getSectionComment(const std::string& sectionName, std::string& comment);

    /**
     * @brief 设置节注释
     * @param sectionName 节名称
     * @param comment 注释
     * @return true-成功, flalse-失败(不存在)
     */
    bool setSectionComment(const std::string& sectionName, const std::string& comment);

    /**
     * @brief 是否存在键
     * @param sectionName 节名称
     * @param key 键
     * @return true-存在, false-不存在
     */
    bool hasKey(const std::string& sectionName, const std::string& key);

    /**
     * @brief 删除键
     * @param nameName 节名称
     * @param key 键
     * @return true-成功, flalse-失败(不存在)
     */
    bool removeKey(const std::string& sectionName, const std::string& key);

    /**
     * @brief 获取值
     * @param sectionName 节名称
     * @param key 键
     * @param value [输出]值
     * @return true-成功, flalse-失败(不存在)
     */
    bool getValue(const std::string& sectionName, const std::string& key, std::string& value);

    /**
     * @brief 设置值
     * @param sectionName 节名称
     * @param key 键
     * @param value 值
     * @return true-成功, false-失败(不存在)
     */
    bool setValue(const std::string& sectionName, const std::string& key, const std::string& value);

    /**
     * @brief 获取注释
     * @param sectionName 节名称
     * @param key 键
     * @param comment [输出]注释
     * @return true-成功, flalse-失败(不存在)
     */
    bool getComment(const std::string& sectionName, const std::string& key, std::string& comment);

    /**
     * @brief 设置注释
     * @param sectionName 节名称
     * @param key 键
     * @param comment 注释
     * @return true-成功, false-失败(不存在)
     */
    bool setComment(const std::string& sectionName, const std::string& key, const std::string& comment);

    /**
     * @brief 是否允许创建节/项
     * @return true-允许, false-不允许
     */
    virtual bool isAllowAutoCreate() = 0;

private:
    /**
     * @brief 判断是否为注释
     * @param str 字符串内容
     * @return true-是, false-否
     */
    bool isComment(const std::string& str);

private:
    std::string m_filename; /* 文件名称 */
    std::map<std::string, std::shared_ptr<IniSection>> m_sections; /* 节列表 */
    std::vector<std::string> m_commentFlags; /* 注释标识列表 */
    bool m_saved = true; /* 是否已保存 */
};
} // namespace ini
