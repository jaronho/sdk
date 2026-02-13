#include "logger.h"

namespace logger
{
Logger::Logger(const std::string& tag, const std::shared_ptr<InnerLogger>& inner) : m_tag(tag), m_inner(inner) {}

bool Logger::isValid() const
{
    if (m_inner)
    {
        return true;
    }
    return false;
}

std::string Logger::getTag() const
{
    return m_tag;
}

std::string Logger::getName() const
{
    if (m_inner)
    {
        return m_inner->getName();
    }
    return std::string();
}

size_t Logger::getMaxSize() const
{
    if (m_inner)
    {
        return m_inner->getMaxSize();
    }
    return 0;
}

void Logger::setMaxSize(size_t maxSize)
{
    if (m_inner)
    {
        m_inner->setMaxSize(maxSize);
    }
}

size_t Logger::getMaxFiles() const
{
    if (m_inner)
    {
        return m_inner->getMaxFiles();
    }
    return 0;
}

void Logger::setMaxFiles(size_t maxFiles)
{
    if (m_inner)
    {
        m_inner->setMaxFiles(maxFiles);
    }
}

int Logger::getLevel() const
{
    if (m_inner)
    {
        return m_inner->getLevel();
    }
    return -1;
}

void Logger::setLevel(int level)
{
    if (m_inner)
    {
        m_inner->setLevel(level);
    }
}

int Logger::getLevelFile(int level) const
{
    if (m_inner)
    {
        return m_inner->getLevelFile(level);
    }
    return -1;
}

void Logger::setLevelFile(int level, int fileType)
{
    if (m_inner)
    {
        m_inner->setLevelFile(level, fileType);
    }
}

int Logger::getFlushLevel() const
{
    if (m_inner)
    {
        return m_inner->getFlushLevel();
    }
    return -1;
}

void Logger::setFlushLevel(int level)
{
    if (m_inner)
    {
        m_inner->setFlushLevel(level);
    }
}

int Logger::getConsoleMode() const
{
    if (m_inner)
    {
        return m_inner->getConsoleMode();
    }
    return 0;
}

void Logger::setConsoleMode(int mode)
{
    if (m_inner)
    {
        m_inner->setConsoleMode(mode);
    }
}

void Logger::trace(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_TRACE, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::trace(const char* file, int line, const char* func, const std::string& msg) const
{
    trace(file, line, func, msg.data(), msg.size());
}

void Logger::debug(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_DEBUG, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::debug(const char* file, int line, const char* func, const std::string& msg) const
{
    debug(file, line, func, msg.data(), msg.size());
}

void Logger::info(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_INFO, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::info(const char* file, int line, const char* func, const std::string& msg) const
{
    info(file, line, func, msg.data(), msg.size());
}

void Logger::warn(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_WARN, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::warn(const char* file, int line, const char* func, const std::string& msg) const
{
    warn(file, line, func, msg.data(), msg.size());
}

void Logger::error(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_ERROR, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::error(const char* file, int line, const char* func, const std::string& msg) const
{
    error(file, line, func, msg.data(), msg.size());
}

void Logger::fatal(const char* file, int line, const char* func, const char* msg, size_t msgLen) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_FATAL, m_tag, file, line, func, msg, msgLen);
    }
}

void Logger::fatal(const char* file, int line, const char* func, const std::string& msg) const
{
    fatal(file, line, func, msg.data(), msg.size());
}

void Logger::forceFlush()
{
    if (m_inner)
    {
        m_inner->forceFlush();
    }
}
} // namespace logger
