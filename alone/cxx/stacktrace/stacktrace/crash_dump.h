#pragma once

#include <functional>
#include <string>
#include <vector>

namespace stacktrace
{
/**
 * @brief 收集结束回调
 * @param filename 崩溃堆栈日志文件名(全路径)
 */
using FinishCallback = std::function<void(const std::string& filename)>;

/**
 * @brief 崩溃收集
 */
class CrashDump final
{
public:
    /**
     * @brief 打开收集功能(需要在程序一启动就打开, 提前于其他逻辑)
     * @param dumpFilePath 崩溃日志文件保存路径
     * @param fullProcName 程序名(全路径)
     * @param callback 崩溃收集后回调, 回调中传崩溃堆栈日志文件名(文件名自动生成)
     */
    static void open(const std::string& dumpFilePath, const std::string& fullProcName, const FinishCallback& callback = nullptr);

private:
    friend void catchHandler(int sigNum);

private:
    static std::string m_dumpFilePath; /* 日志保存路径 */
    static std::string m_fullProcName; /* 当前程序名(全路径) */
    static FinishCallback m_callback; /* 崩溃收集结束回调 */
};
} // namespace stacktrace
