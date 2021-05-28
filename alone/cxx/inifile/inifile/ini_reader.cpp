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

bool IniReader::open(const std::string& filename, size_t lineLength)
{
    m_filename = filename;
    m_lineLength = lineLength;
    int ret = IniFile::open(filename, false, lineLength);
    return 0 == ret;
}

bool IniReader::reload()
{
    int ret = IniFile::open(m_filename, false, m_lineLength);
    return 0 == ret;
}

std::vector<IniSection> IniReader::getDataList()
{
    return IniFile::getDataList();
}

std::vector<std::string> IniReader::getCommentFlags()
{
    return IniFile::getCommentFlags();
}

bool IniReader::hasSection(const std::string& section)
{
    return IniFile::hasSection(section);
}

bool IniReader::getSectionComment(const std::string& section, std::string& comment)
{
    return IniFile::getSectionComment(section, comment);
}

bool IniReader::hasKey(const std::string& section, const std::string& key)
{
    return IniFile::hasKey(section, key);
}

bool IniReader::getComment(const std::string& section, const std::string& key, std::string& comment)
{
    return IniFile::getComment(section, key, comment);
}

bool IniReader::getBool(const std::string& section, const std::string& key, bool defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        std::transform(value.begin(), value.end(), value.begin(), tolower);
        return 0 == value.compare("true");
    }
    return defaultValue;
}

int IniReader::getInt(const std::string& section, const std::string& key, int defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, int>(value);
    }
    return defaultValue;
}

unsigned int IniReader::getUInt(const std::string& section, const std::string& key, unsigned int defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, unsigned int>(value);
    }
    return defaultValue;
}

long IniReader::getLong(const std::string& section, const std::string& key, long defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, long>(value);
    }
    return defaultValue;
}

unsigned long IniReader::getULong(const std::string& section, const std::string& key, unsigned long defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, unsigned long>(value);
    }
    return defaultValue;
}

long long IniReader::getLongLong(const std::string& section, const std::string& key, long long defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, long long>(value);
    }
    return defaultValue;
}

unsigned long long IniReader::getULongLong(const std::string& section, const std::string& key, unsigned long long defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, unsigned long long>(value);
    }
    return defaultValue;
}

float IniReader::getFloat(const std::string& section, const std::string& key, float defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, float>(value);
    }
    return defaultValue;
}

double IniReader::getDouble(const std::string& section, const std::string& key, double defaultValue)
{
    std::string value;
    if (getValue(section, key, value))
    {
        return convert<std::string, double>(value);
    }
    return defaultValue;
}

std::string IniReader::getString(const std::string& section, const std::string& key, std::string defaultValue)
{
    std::string value;
    if (IniFile::getValue(section, key, value))
    {
        return value;
    }
    return defaultValue;
}

bool IniReader::isAllowAutoCreate()
{
    return false;
}
} // namespace ini
