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
 * @brief 打开崩溃监听(需要在程序一启动就打开, 提前于其他逻辑)
 * @param procFile 当前程序文件全路径名
 * @param outputPath 崩溃堆栈文件输出路径
 * @param callback 崩溃回调
 */
void open(const std::string& procFile, const std::string& outputPath, const DumpCallback& callback = nullptr);
} // namespace crashdump
