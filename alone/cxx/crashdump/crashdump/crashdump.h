#pragma once

#include <functional>
#include <string>

namespace crashdump
{
/**
 * @brief 崩溃回调
 * @param fullDumpName 崩溃文件名(全路径)
 */
using DumpCallback = std::function<void(const std::string& fullDumpName)>;

/**
 * @brief 打开崩溃监听(需要在程序一启动就打开, 提前于其他逻辑)
 * @param outputPath 崩溃文件输出路径
 * @param fullProcName 当前程序名(全路径)
 * @param callback 崩溃回调
 */
void open(const std::string& outputPath, const std::string& fullProcName, const DumpCallback& callback = nullptr);
} // namespace crashdump
