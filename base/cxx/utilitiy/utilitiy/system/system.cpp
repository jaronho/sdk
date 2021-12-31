#include "system.h"

#include <chrono>
#include <string.h>
#include <sys/timeb.h>
#include <thread>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace utilitiy
{
int System::runCmd(const std::string& cmd, std::string* outStr, std::vector<std::string>* outVec)
{
    if (outStr)
    {
        (*outStr).clear();
    }
    if (outVec)
    {
        (*outVec).clear();
    }
    if (cmd.empty())
    {
        return -1;
    }
    FILE* stream = NULL;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return -2;
    }
    if (outStr || outVec)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        std::string line;
        while (fread(buffer, 1, bufferSize, stream) > 0)
        {
            if (outStr)
            {
                (*outStr).append(buffer);
            }
            if (outVec)
            {
                line += buffer;
                while (1)
                {
                    size_t pos = line.find("\r\n"), offset = 2;
                    if (std::string::npos == pos)
                    {
                        pos = line.find("\n"), offset = 1;
                    }
                    if (std::string::npos == pos)
                    {
                        break;
                    }
                    (*outVec).emplace_back(line.substr(0, pos));
                    line = line.substr(pos + offset, line.size() - pos - offset);
                }
            }
            memset(buffer, 0, bufferSize);
        }
        if (outVec && !line.empty())
        {
            (*outVec).emplace_back(line);
        }
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    int ret = pclose(stream);
    if (0 != ret && 10 == errno) /* 当进程某处设置了`signal(SIGCHLD, SIG_IGN)`时, 会出现"No child processes", 这里就设置不认为出错 */
    {
        ret = 0;
    }
    return ret;
#endif
}

#ifndef _WIN32
bool System::tryLockFile(int fd, bool lock, bool block)
{
    int lockType = lock ? F_WRLCK : F_UNLCK;
    struct flock fl;
    fl.l_type = lockType; /* 锁的类型 */
    fl.l_whence = SEEK_SET; /* 偏移量的起始位置 */
    fl.l_start = 0; /* 加锁的起始偏移 */
    fl.l_len = 0; /* 上锁的字节数, 为0时表示锁的区域从起点开始直至最大的可能位置 */
    int ret = fcntl(fd, block ? F_SETLKW : F_SETLK, &fl);
    if (0 == ret && lockType == fl.l_type)
    {
        return true;
    }
    return false;
}

bool System::checkFileLock(int fd)
{
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fcntl(fd, F_GETLK, &fl);
    if (F_UNLCK == fl.l_type)
    {
        return false;
    }
    return true;
}

bool System::checkFileLock(const std::string& filename)
{
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) /* 文件不存在 */
    {
        return false;
    }
    bool ret = checkFileLock(fd);
    close(fd);
    return ret;
}
#endif

double System::getTimestamp()
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

System::DateTime System::getDateTime(double timestamp)
{
    long seconds = (long)timestamp;
    int millisecond = (timestamp - (double)seconds) * 1000;
    time_t now = seconds > 0 ? seconds : time(NULL);
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    if (seconds <= 0)
    {
        struct timeb tb;
        ftime(&tb);
        millisecond = tb.millitm;
    }
    /* 填充 */
    DateTime dt;
    dt.year = 1900 + t.tm_year;
    dt.month = 1 + t.tm_mon;
    dt.day = t.tm_mday;
    dt.hour = t.tm_hour;
    dt.minute = t.tm_min;
    dt.second = t.tm_sec;
    dt.millisecond = millisecond;
    dt.wday = t.tm_wday;
    dt.yday = 1 + t.tm_yday;
    return dt;
}

double System::dateTimeToTimestamp(const System::DateTime& dt)
{
    struct tm t;
    t.tm_year = dt.year - 1900;
    t.tm_mon = dt.month - 1;
    t.tm_mday = dt.day;
    t.tm_hour = dt.hour;
    t.tm_min = dt.minute;
    t.tm_sec = dt.second;
    double timestamp = 0;
    timestamp += (long)mktime(&t);
    timestamp += (float)dt.millisecond / 1000;
    return timestamp;
}

bool System::setLocalTime(const System::DateTime& dt)
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

void System::waitForTime(unsigned int maxMS, const std::function<bool()>& func, unsigned int loopGap)
{
    if (func)
    {
        std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxMS);
        while (std::chrono::steady_clock::now() < endTime)
        {
            if (func())
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(maxMS));
    }
}

void System::waitForCount(unsigned int maxCount, const std::function<bool()>& func, unsigned int loopGap)
{
    if (func)
    {
        unsigned int count = 0;
        while (count < maxCount)
        {
            if (func())
            {
                break;
            }
            ++count;
            std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(maxCount * loopGap));
    }
}
} // namespace utilitiy
