#include "logger_manager.h"

#include "impl/inner_logger_impl.h"

#include <assert.h>

namespace logger
{
std::recursive_mutex LoggerManager::m_mutex;
std::string LoggerManager::m_logPath;
std::unordered_map<std::string, InnerLoggerPtr> LoggerManager::m_loggerMap;
std::string LoggerManager::m_defaultLoggerName;
std::string LoggerManager::m_defaultTagName;

void LoggerManager::start(const std::string& logPath, const std::string& defLoggerName, const std::string& defTagName)
{
    assert(!logPath.empty());
    assert(!defLoggerName.empty());
    assert(!defTagName.empty());
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    m_logPath = logPath;
    m_defaultLoggerName = defLoggerName;
    m_defaultTagName = defTagName;
}

Logger LoggerManager::getLogger(const std::string& tagName, const std::string& loggerName)
{
    auto tag = tagName.empty() ? m_defaultTagName : tagName;
    auto name = loggerName.empty() ? m_defaultLoggerName : loggerName;
    std::lock_guard<std::recursive_mutex> locker(m_mutex);
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() != iter)
    {
        return Logger(tag, iter->second);
    }
    /* 默认日志器找不到则创建默认目录下的default日志器 */
    InnerLoggerPtr innerLogger = createInnerLogger(m_logPath, name);
    m_loggerMap.insert(std::make_pair(name, innerLogger));
    return Logger(tag, innerLogger);
}

InnerLoggerPtr LoggerManager::createInnerLogger(const std::string& path, const std::string& name)
{
    assert(!path.empty());
    assert(!name.empty());
    return std::make_shared<InnerLoggerImpl>(path, name);
}
} // namespace logger
