#pragma once
#include <functional>
#include <string>

namespace utilitiy
{
class Process final
{
public:
    /**
     * @brief 获取当前进程id
     * @return 进程id
     */
    static int getProcessId();

    /**
     * @brief 获取当前线程id
     * @return 线程id
     */
    static int getThreadId();

    /**
     * @brief 判断线程id是否有效
     * @param threadId 线程id
     * @return 是否有效
    */
    static bool isValidThreadId(int threadId);

    /**
     * @brief 设置当前线程名
     * @param name 线程名，不要超过15个字符
     */
    static void setThreadName(const std::string& name);

    /**
     * @brief 获取进程的程序全路径文件名
     * @param pid 进程ID(选填), 当<=0时获取当前进程
     * @return 程序全路径文件名, 为空表示进程ID不存在
     */
    static std::string getProcessExeFile(int pid = 0);

    /**
     * @brief 根据程序文件名搜索进程
     * @param filename 程序文件名(可包含全路径), 为空时表示查找所有进程
     * @param callback 匹配时的回调函数, exeFile-程序全路径文件名, pid-匹配到的进程ID, 返回值: true-继续, false-停止
     * @return 匹配到的进程数
     */
    static int searchProcess(const std::string& filename,
                             const std::function<bool(const std::string& exeFile, int pid)>& callback = nullptr);

    /**
     * @brief 启动进程
     * @param exeFile 程序文件名(全路径)
     * @param args 参数
     * @param flag WIN32: 0-不使用新控制台, 1-使用新控制台; Unix: 0-默认子进程标准输出, 1-关闭子进程标准输出
     * @return 进程ID
     */
    static int runProcess(const std::string& exeFile, const std::string& args = "", int flag = 1);

    /**
     * @brief 杀死进程
     * @param pid 进程ID
     * @return true-成功, false-失败
     */
    static bool killProcess(int pid);
};
} // namespace utilitiy
