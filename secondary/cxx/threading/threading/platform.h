#pragma once
#include <string>

namespace threading
{
/**
 * @brief 平台相关的接口集合
 */
class Platform final
{
public:
    /**
     * @brief 获取进程id
     * @return 进程id
     */
    static int64_t getProcessId();

    /**
     * @brief 获取线程id
     * @return 线程id
     */
    static int64_t getThreadId();

    /**
     * @brief 判断线程id是否有效
     * @param threadId 线程id
     * @return 是否有效
    */
    static bool isValidThreadId(int64_t threadId);

    /**
     * @brief 设置线程名
     * @param name 线程名，不要超过15个字符
     */
    static void setThreadName(const std::string& name);
};
} // namespace threading
