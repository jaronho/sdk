#pragma once
#include <functional>
#include <string>

namespace toolkit
{
/**
 * @brief 应用单例类
 */
class AppSingleton final
{
private:
    AppSingleton() = default;

public:
    /**
     * @brief 创建应用单例, 在应用程序主入口建议最先调用
     * @param pidFilePath pid文件路径(选填), 默认在程序文件所在目录
     * @param pidFileName pid文件名(选填), 默认`程序名.pid`
     * @param exitFunc 程序退出前回调(选填), 参数: pidFile-运行中的进程锁文件
     * @return true-成功, false-失败
     */
    static bool create(const std::string& pidFilePath = "", const std::string& pidFileName = "",
                       const std::function<void(const std::string& pidFile)>& exitFunc = nullptr);
};
} // namespace toolkit
