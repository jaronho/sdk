#pragma once
#include <functional>
#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace utility
{
class System final
{
public:
    /**
     * @brief 执行命令
     * @param cmd 命令内容
     * @param outStr [输出]过程输出的可打印内容(选填, 字符串)
     * @param outVec [输出]过程输出的可打印内容(选填, 行列表)
     * @return 执行结果, 0-成功, 非0-失败
     */
    static int runCmd(const std::string& cmd, std::string* outStr = nullptr, std::vector<std::string>* outVec = nullptr);

    /**
     * @brief 对文件加锁/解锁
     * @param fd 文件句柄/描述符(注意: 如果fd被关闭了,则自动解锁)
     * @param lock true-加锁, false-解锁
     * @param block true-阻塞(直到其他进程解锁), false-非阻塞
     * @return true-成功, false-失败
     */
#ifdef _WIN32
    static bool tryLockUnlockFile(HANDLE fd, bool lock, bool block = true);
#else
    static bool tryLockUnlockFile(int fd, bool lock, bool block = true);
#endif

    /**
     * @brief 对文件加锁
     * @param filename 文件名(注意: 内部将会打开1个文件句柄且不会关闭)
     * @param block true-阻塞(直到其他进程解锁), false-非阻塞
     * @return true-成功, false-失败
     */
    static bool tryLockFile(const std::string& filename, bool block = true);

    /**
     * @brief 对文件临时加锁和解锁, 将自动创建名称为`文件名.lock`的加锁文件
     * @param filename 文件名
     * @param func 加解锁期间的执行函数
     * @return true-成功, false-失败
     */
    static bool tryLockFileTemporary(const std::string& filename, const std::function<void()>& func);

    /**
     * @brief 对检测文件是否被加锁
     * @param fd 文件句柄/描述符
     * @return true-被加锁, false-未加锁
     */
#ifdef _WIN32
    static bool checkFileLock(HANDLE fd);
#else
    static bool checkFileLock(int fd);
#endif

    /**
     * @brief 对检测文件是否被加锁
     * @param filename 文件名
     * @return true-被加锁, false-未加锁
     */
    static bool checkFileLock(const std::string& filename);

    /**
     * @brief 等待时间
     * @param maxMS 最大等待的时间(单位:毫秒), 超过此时间则结束等待
     * @param func 过程中执行的函数(选填), 返回值: true-结束等待, false-继续等待
     * @param loopGap 循环的时间间隔(单位:毫秒)(选填)
     */
    static void waitForTime(unsigned int maxMS, const std::function<bool()>& func = nullptr, unsigned int loopGap = 50);

    /**
     * @brief 等待次数
     * @param maxCount 最大等待的次数, 超过此值则结束等待
     * @param func 过程中执行的函数(选填), 返回值: true-结束等待, false-继续等待
     * @param loopGap 循环的时间间隔(单位:毫秒)(选填)
     */
    static void waitForCount(unsigned int maxCount, const std::function<bool()>& func = nullptr, unsigned int loopGap = 50);

    /**
     * @brief 获取主机名
     * @return 主机名
     */
    static std::string getHostname();
};
} // namespace utility
