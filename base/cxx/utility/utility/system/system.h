#pragma once
#include <functional>
#include <string>
#include <vector>

namespace utility
{
class System final
{
public:
    /**
     * @brief 执行命令
     * @param cmd 命令内容, 只捕获stdout, 在Linux平台stderr需在命令后加"2>&1", 还可在命令前加"timeout -k 1 秒 "用命令自身控制超时
     * @param startedFunc 启动回调(选填), 子进程创建成功后调用
     * @param outputFunc 输出回调(选填), 参数: data-数据, count-数据长度, 返回值: true-继续获取数据, false-停止获取数据
     * @param timeoutMS 超时时间(毫秒, 选填), 0-不超时(命令启动的后台进程会持有管道写端, 接口将等到全部写端关闭才返回, 可能永久阻塞, 此类命令应重定向输出或设置超时), 大于0时超时后关闭管道并放弃等待子进程
     * @return 0-成功, -1-命令为空, -2-创建子进程失败, -3-获取描述符失败, -4-超时, -5-回调主动停止; 超时/停止时只关闭管道不结束子进程, 命令可能仍在运行
     */
    static int runCmd(const std::string& cmd, const std::function<void()>& startedFunc,
                      const std::function<bool(const char* data, size_t count)>& outputFunc, unsigned int timeoutMS = 0);

    /**
     * @brief 执行命令
     * @param cmd 命令内容, 只捕获stdout, 在Linux平台stderr需在命令后加"2>&1", 还可在命令前加"timeout -k 1 秒 "用命令自身控制超时
     * @param outStr [输出]全部输出(选填)
     * @param outVec [输出]按行输出(选填)
     * @param ignoreBlankLine 是否忽略空行(选填), 为true时空行将不会添加到outVec行列表中
     * @param timeoutMS 超时时间(毫秒, 选填), 0-不超时(命令启动的后台进程会持有管道写端, 接口将等到全部写端关闭才返回, 可能永久阻塞, 此类命令应重定向输出或设置超时), 大于0时超时后关闭管道并放弃等待子进程
     * @return 0-成功, -1-命令为空, -2-创建子进程失败, -3-获取描述符失败, -4-超时, -5-回调主动停止;
     */
    static int runCmd(const std::string& cmd, std::string* outStr = nullptr, std::vector<std::string>* outVec = nullptr,
                      bool ignoreBlankLine = false, unsigned int timeoutMS = 0);

#ifdef _WIN32
    typedef void* HANDLE;
#else
    typedef int HANDLE;
#endif
    /**
     * @brief 对文件加锁/解锁
     * @param fd 文件句柄/描述符(注意: 如果fd被关闭了,则自动解锁)
     * @param lock true-加锁, false-解锁
     * @param block true-阻塞(直到其他进程解锁), false-非阻塞
     * @return true-成功, false-失败
     */
    static bool tryLockUnlockFile(HANDLE fd, bool lock, bool block = true);

    /**
     * @brief 对文件加锁
     * @param filename 文件名(注意: 内部将会打开1个文件句柄且不会关闭)
     * @param block true-阻塞(直到其他进程解锁), false-非阻塞
     * @return true-成功, false-失败
     */
    static bool tryLockFile(const std::string& filename, bool block = true);

    /**
     * @brief 对文件临时加锁和解锁
     * @param filename 文件名
     * @param func 加解锁期间的执行函数, 参数: lockOk-加锁是否成功
     * @param suffix 加锁文件后缀名(选填), 若为空则将自动创建名称为`文件名.lock`的加锁文件
     * @return true-成功, false-失败
     */
    static bool tryLockFileTemporary(const std::string& filename, const std::function<void(bool lockOk)>& func,
                                     const std::string& suffix = ".lock");

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

#ifdef _WIN32
    /**
     * @brief 判断当前进程是否以管理员权限运行
     * @return true-是, false-否
     */
    static bool isRunAsAdmin();
#endif
};
} // namespace utility
