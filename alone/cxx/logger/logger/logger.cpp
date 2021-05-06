#include "logger.h"

namespace logger
{
Logger::Logger(const std::string& tag, const std::shared_ptr<InnerLogger>& inner) : m_tag(tag), m_inner(inner) {}

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

Level Logger::getLevel() const
{
    if (m_inner)
    {
        return m_inner->getLevel();
    }
    return Level::TRACE;
}

void Logger::setLevel(const Level& level)
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
        m_inner->print(Level::TRACE, m_tag, file, line, func, msg);
    }
}

void Logger::debug(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(Level::DEBUG, m_tag, file, line, func, msg);
    }
}

void Logger::info(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(Level::INFO, m_tag, file, line, func, msg);
    }
}

void Logger::warn(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(Level::WARN, m_tag, file, line, func, msg);
    }
}

void Logger::error(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(Level::ERROR, m_tag, file, line, func, msg);
    }
}

void Logger::fatal(const std::string& file, int line, const std::string& func, const std::string& msg) const
{
    if (m_inner)
    {
        m_inner->print(Level::FATAL, m_tag, file, line, func, msg);
    }
}
} // namespace logger
