#include "daily_logfile.h"

#include <stdio.h>

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
    time_t now;
    time(&now);
    time_t today = (now / 86400); /* 将时间转换为天数整数 */
    if (today != m_today.load(std::memory_order_relaxed)) /* 优先原子判断(避免格式化时间字符串, 避免加锁, 提升性能) */
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        if (today != m_today.load(std::memory_order_relaxed)) /* 双重检查(防止多线程重复重建) */
        {
            struct tm t;
#ifdef _WIN32
            localtime_s(&t, &now);
#else
            t = *localtime(&now);
#endif
            char dateStr[12] = {0};
            strftime(dateStr, sizeof(dateStr), "%Y%m%d", &t);
            m_baseName = m_prefixName + dateStr + m_suffixName;
            std::string path = m_path;
            if (m_createDailyFolder)
            {
                memset(dateStr, 0, sizeof(dateStr));
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &t);
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
