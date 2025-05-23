#pragma once

#include <memory>
#include <mutex>

#include "cfgkey.hpp"
#include "fileparse/ini/ini_writer.h"

/**
 * @brief 配置值对象
 */
class CfgValue
{
public:
    /**
     * @brief 构造函数
     * @param value 值内容
     */
    CfgValue(const std::string& value);

    /**
     * @brief 是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 转为整型
     * @return 整型
     */
    int toInt() const;

    /**
     * @brief 转为长整型
     * @return 长整型
     */
    long long toLongLong() const;

    /**
     * @brief 转为浮点型
     * @return 浮点型
     */
    float toFloat() const;

    /**
     * @brief 转为字符串
     * @return 字符串
     */
    std::string toString() const;

private:
    std::string m_value;
};

/**
 * @brief 配置模块
 */
class Config
{
public:
    /**
     * @brief 初始化
     * @param path 文件路径
     * @param writable 是否可写, 一个配置文件建议只由一个进程写操作(选填), 默认只读
     * @return true-成功, false-失败
     */
    static bool init(const std::string& path, bool writable = false);

    /**
     * @brief 重新从本地加载
     * @return true-成功, false-失败
     */
    static bool reload();

    /**
     * @brief 恢复出厂设置(内部自动保存)
     * @param ignoreSections 要忽略的节
     * @param ignoreKeys 要忽略的项
     * @return true-成功, false-失败
     */
    static bool restoreFactory(const std::vector<std::string>& ignoreSections = {}, const std::vector<std::string>& ignoreKeys = {});

    /**
     * @brief 获取值
     * @param key 名称
     * @return 值对象
     */
    static CfgValue getValue(const std::string& key);

    /**
     * @brief 设置值
     * @param key 名称
     * @param value 整型
     * @return true-成功, false-失败
     */
    static bool setValue(const std::string& key, int value);

    /**
     * @brief 设置值
     * @param key 名称
     * @param value 字符串
     * @return true-成功, false-失败
     */
    static bool setValue(const std::string& key, const std::string& value);

    /**
     * @brief 保存到本地文件
     * @return true-成功, false-失败
     * @return true-成功, false-失败
     */
    static bool save();
};
