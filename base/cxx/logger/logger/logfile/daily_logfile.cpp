#include "daily_logfile.h"

#include <stdio.h>

inline struct tm& getLocalTime()
{
    static thread_local struct tm localTime;
    static thread_local time_t lastSec = 0;
    time_t nowSec = time(nullptr); /* 只获取秒级时间 */
    if (nowSec != lastSec)
    {
        lastSec = nowSec;
#ifdef _WIN32
        localtime_s(&localTime, &nowSec);
#else
        localtime_r(&nowSec, &localTime);
#endif
    }
    return localTime;
}

DailyLogfile::DailyLogfile(const std::string& path, const std::string& prefixName, const std::string& suffixName,
                           const std::string& extName, size_t maxSize, size_t maxFiles, bool indexFixed, bool createDailyFolder)
{
    std::string selfPath, selfName;
    if (path.empty() || prefixName.empty())
    {
        Logfile::getProcessPathAndName(selfPath, selfName);
    }
    m_path = path.empty() ? selfPath : path;
    m_prefixName = prefixName.empty() ? selfName : prefixName;
    m_suffixName = suffixName;
    m_baseName = m_prefixName + m_suffixName;
    m_extName = extName.empty() ? ".log" : extName;
    m_maxSize = maxSize;
    m_maxFiles = maxFiles > 0 ? maxFiles : 0;
    m_indexFixed = indexFixed;
    m_createDailyFolder = createDailyFolder;
}

size_t DailyLogfile::getMaxSize() const
{
    return m_maxSize;
}

void DailyLogfile::setMaxSize(size_t maxSize)
{
    m_maxSize = maxSize;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_rotatingLogfile)
        {
            m_rotatingLogfile->setMaxSize(maxSize);
        }
    }
}

size_t DailyLogfile::getMaxFiles() const
{
    return m_maxFiles;
}

void DailyLogfile::setMaxFiles(size_t maxFiles)
{
    m_maxFiles = maxFiles;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (m_rotatingLogfile)
        {
            m_rotatingLogfile->setMaxFiles(maxFiles);
        }
    }
}

Logfile::Result DailyLogfile::record(const std::string& content, bool newline, bool immediateFlush)
{
    const auto& localTm = getLocalTime();
    auto today = (localTm.tm_year + 1900) * 10000 + (localTm.tm_mon + 1) * 100 + localTm.tm_mday; /* 用本地时间的年月日组合作为天数标识 */
    if (today != m_today.load(std::memory_order_relaxed)) /* 优先原子判断(避免格式化时间字符串, 避免加锁, 提升性能) */
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (today != m_today.load(std::memory_order_relaxed)) /* 双重检查(防止多线程重复重建) */
        {
            char dateStr[12] = {0};
            strftime(dateStr, sizeof(dateStr), "%Y%m%d", &localTm);
            m_baseName = m_prefixName + dateStr + m_suffixName;
            std::string path = m_path;
            if (m_createDailyFolder)
            {
                memset(dateStr, 0, sizeof(dateStr));
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &localTm);
#ifdef _WIN32
                path.append("\\").append(dateStr);
#else
                path.append("/").append(dateStr);
#endif
            }
            m_rotatingLogfile = std::make_shared<RotatingLogfile>(path, m_baseName, m_extName, m_maxSize, m_maxFiles, m_indexFixed);
            m_rotatingLogfile->open();
            m_today.store(today, std::memory_order_release);
        }
    }
    return m_rotatingLogfile->record(content, newline, immediateFlush);
}

bool DailyLogfile::forceFlush()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_rotatingLogfile)
    {
        return m_rotatingLogfile->forceFlush();
    }
    return false;
}
