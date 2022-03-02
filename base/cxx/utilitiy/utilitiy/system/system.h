#pragma once
#include <functional>
#include <string>
#include <vector>

namespace utilitiy
{
class System final
{
public:
    /**
     * @brief 日期
     */
    struct DateTime
    {
        DateTime() {}
        DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond = 0)
            : year(year), month(month), day(day), hour(hour), minute(minute), second(second), millisecond(millisecond)
        {
        }

        int year = 1900; /* 年, 值范围: [1900, ) */
        int month = 1; /* 月, 值范围: [1, 12] */
        int day = 1; /* 日, 值范围: [1, 31] */
        int hour = 0; /* 时, 值范围: [0, 23] */
        int minute = 0; /* 分, 值范围: [0, 59] */
        int second = 0; /* 秒, 值范围: [0, 60] */
        int millisecond = 0; /* 毫秒, 值范围: [0, 999] */
        int wday = 0; /* 在1周中的第几天, 值范围: [0-周日, 1-周一, 2-周二, 3-周三, 4-周四, 5-周五, 6-周六] */
        int yday = 1; /* 在1年中的第几天, 值范围: [1, 366] */
    };

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

    /**
     * @brief 获取当前时间戳(从1970-01-01 00:00:00至今)
     * @return 时间戳(秒)
     */
    static double getTimestamp();

    /**
     * @brief 获取日期
     * @param timestamp 时间戳(选填, 秒数, 从1970-01-01 00:00:00至今), 若为空则获取当前日期
     * @return 日期
     */
    static DateTime getDateTime(double timestamp = 0);

    /**
     * @brief 日期转为时间戳(从1970-01-01 00:00:00至今)
     * @param dt 日期
     * @return 时间戳(秒)
     */
    static double dateTimeToTimestamp(const DateTime& dt);

    /**
     * @brief 设置本地时间
     * @param dt 日期(时间)
     * @return true-成功, false-失败
     */
    static bool setLocalTime(const DateTime& dt);

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
};
} // namespace utilitiy
