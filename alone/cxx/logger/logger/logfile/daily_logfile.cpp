#include "daily_logfile.h"

#include <assert.h>
#include <iostream>
#include <stdio.h>

namespace logger
{
DailyLogfile::DailyLogfile(const std::string& path, const std::string& prefixName, const std::string& extName, size_t maxSize,
                           size_t maxFiles)
{
    assert(!path.empty());
    assert(extName.size() > 1);
    assert(maxSize > 0);
    m_path = path;
    m_prefixName = m_baseName = prefixName;
    m_extName = extName;
    m_maxSize = maxSize;
    m_maxFiles = maxFiles > 0 ? maxFiles : 0;
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
    char dateStr[9] = {0};
    strftime(dateStr, sizeof(dateStr), "%Y%m%d", &t);
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    std::string baseName = m_prefixName + dateStr;
    if (!m_rotatingLogfile || 0 != baseName.compare(m_baseName))
    {
        m_baseName = baseName;
        m_rotatingLogfile = std::make_shared<RotatingLogfile>(m_path, baseName, m_extName, m_maxSize, m_maxFiles);
        m_rotatingLogfile->open();
    }
    return m_rotatingLogfile->record(content, newline);
}
} // namespace logger
