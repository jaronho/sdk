#pragma once
#include <string>
#include <vector>

namespace utilitiy
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

#ifndef _WIN32
    /**
     * @brief 对文件加锁/解锁
     * @param fd 文件描述符(注意: 如果fd被关闭了,则自动解锁)
     * @param lock true-加锁, false-解锁
     * @param block true-阻塞(直到其他进程解锁), false-非阻塞
     * @return true-成功, false-失败
     */
    static bool tryLockFile(int fd, bool lock, bool block = true);

    /**
     * @brief 对检测文件是否被加锁
     * @param fd 文件描述符
     * @return true-被加锁, false-未加锁
     */
    static bool checkFileLock(int fd);

    /**
     * @brief 对检测文件是否被加锁
     * @param filename 文件名
     * @return true-被加锁, false-未加锁
     */
    static bool checkFileLock(const std::string& filename);
#endif
};
} // namespace utilitiy
