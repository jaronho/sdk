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
        throw std::exception(std::logic_error("arg 'cfg.path' is empty"));
    }
    if (cfg.name.empty())
    {
        throw std::exception(std::logic_error("arg 'cfg.name' is empty"));
    }
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logCfg = cfg;
    m_defaultTagName = defultTagName;
}

Logger LoggerManager::getLogger(const std::string& tagName, const std::string& loggerName)
{
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto tag = tagName.empty() ? m_defaultTagName : tagName;
    std::lock_guard<std::mutex> locker(m_mutex);
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() != iter)
    {
        return Logger(tag, iter->second);
    }
    /* 默认日志器找不到则创建 */
    if (m_logCfg.path.empty())
    {
        throw std::exception(std::logic_error("var 'm_logCfg.path' is empty"));
    }
    LogConfig cfg = m_logCfg;
    cfg.name = name;
    InnerLoggerPtr innerLogger = createInnerLogger(cfg);
    m_loggerMap.insert(std::make_pair(name, innerLogger));
    return Logger(tag, innerLogger);
}

InnerLoggerPtr LoggerManager::createInnerLogger(const LogConfig& cfg)
{
    return std::make_shared<InnerLoggerImpl>(cfg);
}
} // namespace logger
