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

Logfile::Result DailyLogfile::record(const std::string& content, bool newline)
{
    struct tm t;
    time_t now;
    time(&now);
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    char dateStr[12] = {0};
    strftime(dateStr, sizeof(dateStr), "%Y%m%d", &t);
    std::lock_guard<std::mutex> locker(m_mutex);
    std::string baseName = m_prefixName + dateStr + m_suffixName;
    if (!m_rotatingLogfile || 0 != baseName.compare(m_baseName))
    {
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
        m_baseName = baseName;
        m_rotatingLogfile = std::make_shared<RotatingLogfile>(path, baseName, m_extName, m_maxSize, m_maxFiles, m_indexFixed);
        m_rotatingLogfile->open();
    }
    return m_rotatingLogfile->record(content, newline);
}
