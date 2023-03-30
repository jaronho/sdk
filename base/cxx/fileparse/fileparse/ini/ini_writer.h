#pragma once
#include "ini_reader.h"

namespace ini
{
/**
 * @brief INI文件读写器
 */
class IniWriter final : public IniReader
{
public:
    virtual ~IniWriter() = default;

    /**
     * @brief 打开文件
     * @param filename 文件名, 例如: "test.ini", "../test.ini", "temp\\test.ini"
     * @param errorDesc [输出]错误信息
     * @return 0-成功, 1-文件无法打开, 2-不匹配']', 3-节名称为空, 4-节名称重复, 5-项解析出错, 6-键名称重复
     */
    int open(const std::string& filename, std::string& errorDesc);

    /**
     * @brief 设置允许自动创建节/项
     */
    void setAllowAutoCreate();

    /**
     * @brief 保存文件
     * @param sortType 排序类型: 0-默认, 1-升序, 2-降序
     * @return 0-成功, 1-未变更, 2-文件打开失败, 3-写文件失败
     */
    int save(int sortType = 0);

    /**
     * @brief 清除
     */
    void clear();

    /**
     * @brief 设置注释标识符列表
     * @param flags 标识符列表
     * @return true-成功, flalse-失败
     */
    bool setCommentFlags(const std::vector<std::string>& flags);

    /**
     * @brief 删除节
     * @param nameName 节名称
     * @return true-成功, flalse-失败(不存在)
     */
    bool removeSection(const std::string& name);

    /**
     * @brief 设置节注释
     * @param name 节名称
     * @param comment 注释
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setSectionComment(const std::string& name, const std::string& comment);

    /**
     * @brief 删除项
     * @param nameName 节名称
     * @param key 键
     * @return true-成功, flalse-失败(不存在)
     */
    bool removeItem(const std::string& name, const std::string& key);

    /**
     * @brief 设置项注释
     * @param name 节名称
     * @param key 键
     * @param comment 注释
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setComment(const std::string& name, const std::string& key, const std::string& comment);

    /**
     * @brief 设置布尔型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, bool value);

    /**
     * @brief 设置整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, int value);

    /**
     * @brief 设置无符号整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, unsigned int value);

    /**
     * @brief 设置长整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, long value);

    /**
     * @brief 设置无符号长整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, unsigned long value);

    /**
     * @brief 设置64位长整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, long long value);

    /**
     * @brief 设置无符号64位长整型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, unsigned long long value);

    /**
     * @brief 设置单精度浮点型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, float value);

    /**
     * @brief 设置双精度浮点型
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, double value);

    /**
     * @brief 设置字符串
     * @param name 节名称
     * @param key 键
     * @param value 值
     * @return 0-成功, 1-未变更, 2-参数错误, 3-不允许自动创建
     */
    int setValue(const std::string& name, const std::string& key, std::string value);

    /**
     * @brief 是否允许创建节/项
     * @return true-允许, false-不允许
     */
    bool isAllowAutoCreate() const override;

private:
    bool m_allowAutoCreate = false; /* 是否允许自动创建节/项 */
};
} // namespace ini
