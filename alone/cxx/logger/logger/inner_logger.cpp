#include "inner_logger.h"

namespace logger
{
InnerLogger::InnerLogger(const std::string& path, const std::string& name) : m_path(path), m_name(name) {}

std::string InnerLogger::getPath() const
{
    return m_path;
}

std::string InnerLogger::getName() const
{
    return m_name;
}
} // namespace logger
