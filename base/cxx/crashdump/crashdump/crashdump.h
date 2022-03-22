#pragma once

#include <functional>
#include <string>

namespace crashdump
{
/**
 * @brief 崩溃回调
 * @param json json字符串, 格式如: {"code": 0, "file": "/home/dump/test_20210609135634409.txt", "msg": "ok"}
 */
using DumpCallback = std::function<void(const std::string& json)>;

/**
 * @brief 开始崩溃监听(需要在程序一启动就开始, 提前于其他逻辑)
 * @param outputPath 崩溃堆栈文件输出路径
 * @param callback 崩溃回调(选填), 当非空时, 将把堆栈文件回调出去
 */
void start(const std::string& outputPath, const DumpCallback& callback = nullptr);

/**
 * @brief 设置程序版本(用于拼接再崩溃堆栈文件名上)
 * @param procVersion 程序版本
 */
void setProcVersion(const std::string& procVersion);
} // namespace crashdump
