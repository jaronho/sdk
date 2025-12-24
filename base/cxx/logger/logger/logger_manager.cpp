#include "logger_manager.h"

#include <stdexcept>

#include "impl/inner_logger_impl.h"

namespace logger
{
std::mutex LoggerManager::m_mutex;
LogConfig LoggerManager::m_logCfg;
std::unordered_map<std::string, InnerLoggerPtr> LoggerManager::m_loggerMap;
std::string LoggerManager::m_defaultTagName;

void LoggerManager::setConfig(const LogConfig& cfg, const std::string& defultTagName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_logCfg = cfg;
    m_defaultTagName = defultTagName;
    for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
    {
        iter->second->setMaxSize(cfg.fileMaxSize);
        iter->second->setMaxFiles(cfg.fileMaxCount);
        iter->second->setLevel(cfg.level);
        for (const auto& kv : cfg.levelFile)
        {
            iter->second->setLevelFile(kv.first, kv.second);
        }
        iter->second->setConsoleMode(cfg.consoleMode);
    }
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
    LogConfig cfg = m_logCfg;
    std::string selfPath, selfName;
    if (cfg.path.empty() || cfg.name.empty())
    {
        Logfile::getProcessPathAndName(selfPath, selfName);
    }
    cfg.path = cfg.path.empty() ? selfPath : cfg.path;
    cfg.name = cfg.name.empty() ? selfName : cfg.name;
    cfg.fileExtName = cfg.fileExtName.empty() ? ".log" : cfg.fileExtName;
    cfg.level = level < 0 ? cfg.level : level;
    InnerLoggerPtr innerLogger = createInnerLogger(cfg);
    m_loggerMap.insert(std::make_pair(name, innerLogger));
    return Logger(tag, innerLogger);
}

size_t LoggerManager::getFileMaxSize(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        return m_logCfg.fileMaxSize;
    }
    return iter->second->getMaxSize();
}

void LoggerManager::setFileMaxSize(size_t maxSize, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    maxSize = maxSize <= 0 ? m_logCfg.fileMaxSize : maxSize;
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setMaxSize(maxSize);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setMaxSize(maxSize);
        }
    }
}

size_t LoggerManager::getFileMaxCount(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        return m_logCfg.fileMaxCount;
    }
    return iter->second->getMaxFiles();
}

void LoggerManager::setFileMaxCount(size_t maxCount, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    maxCount = maxCount <= 0 ? m_logCfg.fileMaxCount : maxCount;
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setMaxFiles(maxCount);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setMaxFiles(maxCount);
        }
    }
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

int LoggerManager::getLevelFile(int level, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        auto it = m_logCfg.levelFile.find(level);
        return (m_logCfg.levelFile.end() == it ? -1 : it->second);
    }
    return iter->second->getLevelFile(level);
}

void LoggerManager::setLevelFile(int level, int fileType, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setLevelFile(level, fileType);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setLevelFile(level, fileType);
        }
    }
}

int LoggerManager::getFlushLevel(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        return m_logCfg.flushLevel;
    }
    return iter->second->getFlushLevel();
}

void LoggerManager::setFlushLevel(int level, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    level = level < 0 ? m_logCfg.flushLevel : level;
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setFlushLevel(level);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setFlushLevel(level);
        }
    }
}

int LoggerManager::getConsoleMode(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    auto name = loggerName.empty() ? m_logCfg.name : loggerName;
    auto iter = m_loggerMap.find(name);
    if (m_loggerMap.end() == iter)
    {
        return m_logCfg.consoleMode;
    }
    return iter->second->getConsoleMode();
}

void LoggerManager::setConsoleMode(int mode, const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->setConsoleMode(mode);
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->setConsoleMode(mode);
        }
    }
}

void LoggerManager::forceFlush(const std::string& loggerName)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (loggerName.empty())
    {
        for (auto iter = m_loggerMap.begin(); m_loggerMap.end() != iter; ++iter)
        {
            iter->second->forceFlush();
        }
    }
    else
    {
        auto iter = m_loggerMap.find(loggerName);
        if (m_loggerMap.end() != iter)
        {
            iter->second->forceFlush();
        }
    }
}

InnerLoggerPtr LoggerManager::createInnerLogger(const LogConfig& cfg)
{
    return std::make_shared<InnerLoggerImpl>(cfg);
}
} // namespace logger
