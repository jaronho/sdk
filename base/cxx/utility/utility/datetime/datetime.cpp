#include "datetime.h"

#include <math.h>
#include <sys/timeb.h>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

namespace utility
{
DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
    : year(year), month(month), day(day), hour(hour), minute(minute), second(second), millisecond(millisecond)
{
}

DateTime::DateTime(int hour, int minute, int second, int millisecond) : hour(hour), minute(minute), second(second), millisecond(millisecond)
{
    time_t now = time(NULL);
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    /* 填充 */
    year = 1900 + t.tm_year;
    month = 1 + t.tm_mon;
    day = t.tm_mday;
    wday = t.tm_wday;
    yday = 1 + t.tm_yday;
}

DateTime::DateTime(double timestamp)
{
    long secs = (long)timestamp;
    int ms = (int)roundf((timestamp - (double)secs) * 1000);
    time_t now = secs > 0 ? secs : time(NULL);
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    if (secs <= 0) /* 未指定时间戳, 获取当前时间毫秒 */
    {
        struct timeb tb;
        ftime(&tb);
        ms = tb.millitm;
    }
    /* 填充 */
    year = 1900 + t.tm_year;
    month = 1 + t.tm_mon;
    day = t.tm_mday;
    hour = t.tm_hour;
    minute = t.tm_min;
    second = t.tm_sec;
    millisecond = ms;
    wday = t.tm_wday;
    yday = 1 + t.tm_yday;
}

DateTime::DateTime(const std::string& dtString, const char sep1[1], const char sep2[1], const char sep3[1])
{
    std::string sep1Str = sep1[0] ? std::string(1, sep1[0]) : "";
    std::string sep2Str = sep2[0] ? std::string(1, sep2[0]) : "";
    std::string sep3Str = sep3[0] ? std::string(1, sep3[0]) : "";
    std::string fmtStr = "%04d" + sep1Str + "%02d" + sep1Str + "%02d" + sep2Str + "%02d" + sep3Str + "%02d" + sep3Str + "%02d";
#ifdef _WIN32
    sscanf_s(dtString.c_str(), fmtStr.c_str(), &year, &month, &day, &hour, &minute, &second);
#else
    sscanf(dtString.c_str(), fmtStr.c_str(), &year, &month, &day, &hour, &minute, &second);
#endif
}

bool DateTime::operator==(const DateTime& other) const
{
    return (year == other.year && month == other.month && day == other.day && hour == other.hour && minute == other.minute
            && second == other.second);
}

bool DateTime::operator!=(const DateTime& other) const
{
    return !(other == *this);
}

bool DateTime::operator>(const DateTime& other) const
{
    do
    {
        if (year > other.year)
        {
            return true;
        }
        else if (year < other.year)
        {
            break;
        }
        if (month > other.month)
        {
            return true;
        }
        else if (month < other.month)
        {
            break;
        }
        if (day > other.day)
        {
            return true;
        }
        else if (day < other.day)
        {
            break;
        }
        if (hour > other.hour)
        {
            return true;
        }
        else if (hour < other.hour)
        {
            break;
        }
        if (minute > other.minute)
        {
            return true;
        }
        else if (minute < other.minute)
        {
            break;
        }
        if (second > other.second)
        {
            return true;
        }
        else if (second < other.second)
        {
            break;
        }
    } while (0);
    return false;
}

bool DateTime::operator>=(const DateTime& other) const
{
    do
    {
        if (year > other.year)
        {
            return true;
        }
        else if (year < other.year)
        {
            break;
        }
        if (month > other.month)
        {
            return true;
        }
        else if (month < other.month)
        {
            break;
        }
        if (day > other.day)
        {
            return true;
        }
        else if (day < other.day)
        {
            break;
        }
        if (hour > other.hour)
        {
            return true;
        }
        else if (hour < other.hour)
        {
            break;
        }
        if (minute > other.minute)
        {
            return true;
        }
        else if (minute < other.minute)
        {
            break;
        }
        if (second > other.second)
        {
            return true;
        }
        else if (second < other.second)
        {
            break;
        }
        return true;
    } while (0);
    return false;
}

bool DateTime::operator<(const DateTime& other) const
{
    do
    {
        if (year < other.year)
        {
            return true;
        }
        else if (year > other.year)
        {
            break;
        }
        if (month < other.month)
        {
            return true;
        }
        else if (month > other.month)
        {
            break;
        }
        if (day < other.day)
        {
            return true;
        }
        else if (day > other.day)
        {
            break;
        }
        if (hour < other.hour)
        {
            return true;
        }
        else if (hour > other.hour)
        {
            break;
        }
        if (minute < other.minute)
        {
            return true;
        }
        else if (minute > other.minute)
        {
            break;
        }
        if (second < other.second)
        {
            return true;
        }
        else if (second > other.second)
        {
            break;
        }
    } while (0);
    return false;
}

bool DateTime::operator<=(const DateTime& other) const
{
    do
    {
        if (year < other.year)
        {
            return true;
        }
        else if (year > other.year)
        {
            break;
        }
        if (month < other.month)
        {
            return true;
        }
        else if (month > other.month)
        {
            break;
        }
        if (day < other.day)
        {
            return true;
        }
        else if (day > other.day)
        {
            break;
        }
        if (hour < other.hour)
        {
            return true;
        }
        else if (hour > other.hour)
        {
            break;
        }
        if (minute < other.minute)
        {
            return true;
        }
        else if (minute > other.minute)
        {
            break;
        }
        if (second < other.second)
        {
            return true;
        }
        else if (second > other.second)
        {
            break;
        }
        return true;
    } while (0);
    return false;
}

void DateTime::reset()
{
    year = 1900;
    month = 1;
    day = 1;
    hour = 0;
    minute = 0;
    second = 0;
    millisecond = 0;
    wday = 0;
    yday = 1;
}

bool DateTime::isValid() const
{
    return ((year >= 1900) && (month >= 1 && month <= 12) && (day >= 1 && day <= 31) && (hour >= 0 && hour <= 23)
            && (minute >= 0 && minute <= 59) && (second >= 0 && second <= 60) && (millisecond >= 0 && millisecond <= 999)
            && (wday >= 0 && wday <= 6) && (yday >= 1 && yday <= 366));
}

double DateTime::toTimestamp() const
{
    struct tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    double timestamp = 0;
    timestamp += (long)mktime(&t);
    timestamp += (float)millisecond / 1000;
    return (timestamp > 0 ? timestamp : 0);
}

std::string DateTime::yyyyMMdd(const char sep[1]) const
{
    std::string sepStr = (sep && sep[0]) ? std::string(1, sep[0]) : "";
    std::string fmtStr = "%04d" + sepStr + "%02d" + sepStr + "%02d";
    char buf[11] = {0};
#ifdef _WIN32
    sprintf_s(buf, sizeof(buf), fmtStr.c_str(), year, month, day);
#else
    sprintf(buf, fmtStr.c_str(), year, month, day);
#endif
    return buf;
}

std::string DateTime::hhmm(const char sep1[1]) const
{
    std::string sep1Str = (sep1 && sep1[0]) ? std::string(1, sep1[0]) : "";
    std::string fmtStr = "%02d" + sep1Str + "%02d";
    char buf[6] = {0};
#ifdef _WIN32
    sprintf_s(buf, sizeof(buf), fmtStr.c_str(), hour, minute);
#else
    sprintf(buf, fmtStr.c_str(), hour, minute);
#endif
    return buf;
}

std::string DateTime::hhmmss(const char sep1[1], const char sep2[1]) const
{
    std::string sep1Str = (sep1 && sep1[0]) ? std::string(1, sep1[0]) : "";
    std::string fmtStr = "%02d" + sep1Str + "%02d" + sep1Str + "%02d";
    char buf[13] = {0};
    if (sep2)
    {
        std::string sep2Str = sep2[0] ? std::string(1, sep2[0]) : "";
        fmtStr += sep2Str + "%03d";
#ifdef _WIN32
        sprintf_s(buf, sizeof(buf), fmtStr.c_str(), hour, minute, second, millisecond);
#else
        sprintf(buf, fmtStr.c_str(), hour, minute, second, millisecond);
#endif
    }
    else
    {
#ifdef _WIN32
        sprintf_s(buf, sizeof(buf), fmtStr.c_str(), hour, minute, second);
#else
        sprintf(buf, fmtStr.c_str(), hour, minute, second);
#endif
    }
    return buf;
}

std::string DateTime::yyyyMMddhhmm(const char sep1[1], const char sep2[1], const char sep3[1]) const
{
    std::string sep1Str = (sep1 && sep1[0]) ? std::string(1, sep1[0]) : "";
    std::string sep2Str = (sep2 && sep2[0]) ? std::string(1, sep2[0]) : "";
    std::string sep3Str = (sep3 && sep3[0]) ? std::string(1, sep3[0]) : "";
    std::string fmtStr = "%04d" + sep1Str + "%02d" + sep1Str + "%02d" + sep2Str + "%02d" + sep3Str + "%02d";
    char buf[17] = {0};
#ifdef _WIN32
    sprintf_s(buf, sizeof(buf), fmtStr.c_str(), year, month, day, hour, minute);
#else
    sprintf(buf, fmtStr.c_str(), year, month, day, hour, minute);
#endif
    return buf;
}

std::string DateTime::yyyyMMddhhmmss(const char sep1[1], const char sep2[1], const char sep3[1], const char sep4[1]) const
{
    std::string sep1Str = (sep1 && sep1[0]) ? std::string(1, sep1[0]) : "";
    std::string sep2Str = (sep2 && sep2[0]) ? std::string(1, sep2[0]) : "";
    std::string sep3Str = (sep3 && sep3[0]) ? std::string(1, sep3[0]) : "";
    std::string fmtStr = "%04d" + sep1Str + "%02d" + sep1Str + "%02d" + sep2Str + "%02d" + sep3Str + "%02d" + sep3Str + "%02d";
    char buf[24] = {0};
    if (sep4)
    {
        std::string sep4Str = sep4[0] ? std::string(1, sep4[0]) : "";
        fmtStr += sep4Str + "%03d";
#ifdef _WIN32
        sprintf_s(buf, sizeof(buf), fmtStr.c_str(), year, month, day, hour, minute, second, millisecond);
#else
        sprintf(buf, fmtStr.c_str(), year, month, day, hour, minute, second, millisecond);
#endif
    }
    else
    {
#ifdef _WIN32
        sprintf_s(buf, sizeof(buf), fmtStr.c_str(), year, month, day, hour, minute, second);
#else
        sprintf(buf, fmtStr.c_str(), year, month, day, hour, minute, second);
#endif
    }
    return buf;
}

DateTime DateTime::getNow()
{
    return DateTime(0);
}

double DateTime::getNowTimestamp()
{
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    /* Windows file time (time since January 1, 1601 (UTC)) */
    double t = ft.dwLowDateTime / 1.0e7 + ft.dwHighDateTime * (4294967296.0 / 1.0e7);
    /* convert to Unix Epoch time (time since January 1, 1970 (UTC)) */
    return (t - 11644473600.0);
#else
    struct timeval v;
    gettimeofday(&v, (struct timezone*)NULL);
    /* Unix Epoch time (time since January 1, 1970 (UTC)) */
    return v.tv_sec + v.tv_usec / 1.0e6;
#endif
}

bool DateTime::setLocalTime(const DateTime& dt)
{
#ifdef _WIN32
    SYSTEMTIME tv;
    tv.wYear = dt.year;
    tv.wMonth = dt.month;
    tv.wDay = dt.day;
    tv.wHour = dt.hour;
    tv.wMinute = dt.minute;
    tv.wSecond = dt.second;
    if (SetLocalTime(&tv))
    {
        return true;
    }
#else
    struct tm t;
    t.tm_year = dt.year - 1900;
    t.tm_mon = dt.month - 1;
    t.tm_mday = dt.day;
    t.tm_hour = dt.hour;
    t.tm_min = dt.minute;
    t.tm_sec = dt.second;
    struct timeval tv;
    tv.tv_sec = mktime(&t);
    tv.tv_usec = 0;
    if (0 == settimeofday(&tv, NULL))
    {
        return true;
    }
#endif
    return false;
}
} // namespace utility
