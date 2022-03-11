#pragma once
#include <string>

namespace utility
{
/**
 * @brief 日期
 */
class DateTime
{
public:
    DateTime() = default;

    /**
     * @brief 构造函数
     * @param 年月日时分秒[毫秒]
     */
    DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond = 0);

    /**
     * @brief 构造函数
     * @param 时分秒[毫秒]
     */
    DateTime(int hour, int minute, int second, int millisecond = 0);

    /**
     * @brief 构造函数
     * @param timestamp 时间戳(秒)(从1970-01-01 00:00:00至今), 若为0则表示获取当前时间
     */
    DateTime(double timestamp);

    /**
     * @brief 构造函数
     * @param dtString 年月日时分秒格式化字符串, 例如: "2022-12-03 12:32:03"
     * @param sep1 分隔符(选填), 例如: "-"
     * @param sep2 分隔符(选填), 例如: " "
     * @param sep3 分隔符(选填), 例如: ":"
     */
    DateTime(const std::string& dtString, const char sep1[1] = "", const char sep2[1] = "", const char sep3[1] = "");

    bool operator==(const DateTime& other) const;
    bool operator!=(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator<=(const DateTime& other) const;

    /**
     * @brief 重置日期
     */
    void reset();

    /**
     * @brief 判断日期是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 转为时间戳(从1970-01-01 00:00:00至今)
     * @return 时间戳(秒)
     */
    double toTimestamp() const;

    /**
     * @brief 年月日格式化字符串
     * @param sep 分隔符(选填), 例如: "-"
     * @return 字符串, 例如: "2022-12-03"
     */
    std::string yyyyMMdd(const char sep[1] = "") const;

    /**
     * @brief 时分秒格式化字符串
     * @param sep 分隔符(选填), 例如: ":"
     * @return 字符串, 例如: "12:32:03"
     */
    std::string hhmmss(const char sep[1] = "") const;

    /**
     * @brief 年月日时分秒格式化字符串
     * @param sep1 分隔符(选填), 例如: "-"
     * @param sep2 分隔符(选填), 例如: " "
     * @param sep3 分隔符(选填), 例如: ":"
     * @return 字符串, 例如: "2022-12-03 12:32:03"
     */
    std::string yyyyMMddhhmmss(const char sep1[1] = "", const char sep2[1] = "", const char sep3[1] = "") const;

    /**
     * @brief 获取当前日期
     * @return 当前日期
     */
    static DateTime getNow();

    /**
     * @brief 获取当前时间戳(从1970-01-01 00:00:00至今)
     * @return 当前时间戳(秒)
     */
    static double getNowTimestamp();

    /**
     * @brief 设置本地时间
     * @param dt 日期
     * @return true-成功, false-失败
     */
    static bool setLocalTime(const DateTime& dt);

public:
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
} // namespace utility
