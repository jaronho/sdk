#include "logger.h"

namespace logger
{
Logger::Logger(const std::string& tag, const std::shared_ptr<InnerLogger>& inner) : m_tag(tag), m_inner(inner) {}

Logger& Logger::operator=(const Logger& src)
{
    m_tag = src.m_tag;
    m_inner = src.m_inner;
    return (*this);
}

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

int Logger::getLevel() const
{
    if (m_inner)
    {
        return m_inner->getLevel();
    }
    return LEVEL_TRACE;
}

void Logger::setLevel(int level)
{
    if (m_inner)
    {
        m_inner->setLevel(level);
    }
}

bool Logger::isConsoleEnable()
{
    if (m_inner)
    {
        return m_inner->isConsoleEnable();
    }
    return true;
}

void Logger::setConsoleEnable(bool enable)
{
    if (m_inner)
    {
        m_inner->setConsoleEnable(enable);
    }
}

void Logger::trace(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_TRACE, m_tag, file, line, func, msg);
    }
}

void Logger::debug(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_DEBUG, m_tag, file, line, func, msg);
    }
}

void Logger::info(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_INFO, m_tag, file, line, func, msg);
    }
}

void Logger::warn(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_WARN, m_tag, file, line, func, msg);
    }
}

void Logger::error(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_ERROR, m_tag, file, line, func, msg);
    }
}

void Logger::fatal(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(LEVEL_FATAL, m_tag, file, line, func, msg);
    }
}
} // namespace logger
