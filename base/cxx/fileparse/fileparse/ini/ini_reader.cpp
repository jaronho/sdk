#include "ini_reader.h"

#include <algorithm>
#include <sstream>

namespace ini
{
template<typename in_value, typename out_type>
out_type convert(const in_value& t)
{
    std::stringstream ss;
    ss << t;
    out_type result;
    ss >> result;
    return result;
}

int IniReader::open(const std::string& filename, std::string& errorDesc)
{
    m_filename = filename;
    return IniFile::open(filename, false, errorDesc);
}

int IniReader::reload(std::string& errorDesc)
{
    return IniFile::open(m_filename, false, errorDesc);
}

std::vector<IniSection> IniReader::getSections() const
{
    return IniFile::getSections();
}

std::vector<std::string> IniReader::getCommentFlags() const
{
    return IniFile::getCommentFlags();
}

bool IniReader::hasSection(const std::string& name) const
{
    return IniFile::hasSection(name);
}

bool IniReader::getSectionComment(const std::string& name, std::string& comment) const
{
    return IniFile::getSectionComment(name, comment);
}

bool IniReader::hasItem(const std::string& name, const std::string& key) const
{
    return IniFile::hasItem(name, key);
}

bool IniReader::getComment(const std::string& name, const std::string& key, std::string& comment) const
{
    return IniFile::getComment(name, key, comment);
}

bool IniReader::getBool(const std::string& name, const std::string& key, bool defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        std::transform(value.begin(), value.end(), value.begin(), tolower);
        return 0 == value.compare("true");
    }
    return defaultValue;
}

int IniReader::getInt(const std::string& name, const std::string& key, int defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, int>(value);
    }
    return defaultValue;
}

unsigned int IniReader::getUInt(const std::string& name, const std::string& key, unsigned int defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, unsigned int>(value);
    }
    return defaultValue;
}

long IniReader::getLong(const std::string& name, const std::string& key, long defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, long>(value);
    }
    return defaultValue;
}

unsigned long IniReader::getULong(const std::string& name, const std::string& key, unsigned long defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, unsigned long>(value);
    }
    return defaultValue;
}

long long IniReader::getLongLong(const std::string& name, const std::string& key, long long defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, long long>(value);
    }
    return defaultValue;
}

unsigned long long IniReader::getULongLong(const std::string& name, const std::string& key, unsigned long long defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, unsigned long long>(value);
    }
    return defaultValue;
}

float IniReader::getFloat(const std::string& name, const std::string& key, float defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, float>(value);
    }
    return defaultValue;
}

double IniReader::getDouble(const std::string& name, const std::string& key, double defaultValue) const
{
    std::string value;
    if (getValue(name, key, value))
    {
        return convert<std::string, double>(value);
    }
    return defaultValue;
}

std::string IniReader::getString(const std::string& name, const std::string& key, std::string defaultValue) const
{
    std::string value;
    if (IniFile::getValue(name, key, value))
    {
        return value;
    }
    return defaultValue;
}

bool IniReader::isAllowAutoCreate() const
{
    return false;
}
} // namespace ini
