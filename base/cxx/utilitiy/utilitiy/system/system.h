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
     * @param result [输出]过程输出的可打印内容(选填, 行列表)
     * @return 执行结果, 0-成功, 非0-失败
     */
    static int runCmd(const std::string& cmd, std::vector<std::string>* result = nullptr);

#ifndef _WIN32
    /**
     * @brief 对文件加锁/解锁
     * @param fd 文件描述符
     * @param lock true-加锁, false-解锁
     * @return -1-出错, 其他值-正确
     */
    static int tryLockFile(int fd, bool lock);

    /**
     * @brief 对文件加锁/解锁
     * @param filename 文件名
     * @param lock true-加锁, false-解锁
     * @return -1-出错, 其他值-正确
     */
    static int tryLockFile(const std::string& filename, bool lock);

    /**
     * @brief 获取文件的加锁状态
     * @param filename 文件名
     * @param lock true-加锁, false-解锁
     * @return 1-文件不存在, 2-文件已被加锁, 3-文件未被加锁
     */
    static int getFileLockStatus(const std::string& filename);
#endif
};
} // namespace utilitiy
