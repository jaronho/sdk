#include "logger_manager.h"

#include <stdexcept>

#include "impl/inner_logger_impl.h"

namespace logger
{
std::mutex LoggerManager::m_mutex;
LogConfig LoggerManager::m_logCfg;
std::unordered_map<std::string, InnerLoggerPtr> LoggerManager::m_loggerMap;
std::string LoggerManager::m_defaultTagName;

void LoggerManager::start(const LogConfig& cfg, const std::string& defultTagName)
{
    if (cfg.path.empty())
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] arg 'cfg.path' is empty");
    }
    if (cfg.name.empty())
    {
        throw std::logic_error(std::string("[") + __FILE__ + " " + std::to_string(__LINE__) + " " + __FUNCTION__
                               + "] arg 'cfg.name' is empty");
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logCfg = cfg;
    m_defaultTagName = defultTagName;
}

Logger LoggerManager::getLogger(const std::string& tagName, int level, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto tag = tagName.empty() ? m_defaultTagName : tagName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() != iter)
    {
        return Logger(tag, iter->second);
    }
    /* 默认日志器找不到则创建 */
    if (m_logCfg.path.empty())
    {
        return Logger();
    }
    LogConfig cfg = m_logCfg;
    cfg.name = name;
    cfg.level = level < 0 ? cfg.level : level;
    InnerLoggerPtr innerLogger = createInnerLogger(cfg);
    m_loggerMap.insert(std::make_pair(name, innerLogger));
    return Logger(tag, innerLogger);
}

int LoggerManager::getLevel(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        return m_logCfg.level;
    }
    return iter->second->getLevel();
}

void LoggerManager::setLevel(int level, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    level = level < 0 ? m_logCfg.level : level;
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setLevel(level);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setLevel(level);
        }
    }
}

InnerLoggerPtr LoggerManager::createInnerLogger(const LogConfig& cfg)
{
    return std::make_shared<InnerLoggerImpl>(cfg);
}
} // namespace logger
