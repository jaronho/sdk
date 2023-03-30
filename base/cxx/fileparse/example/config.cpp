#include "config.h"

#include <algorithm>

static std::mutex s_mutex;
static std::shared_ptr<ini::IniWriter> s_iniWriter = nullptr; /* 配置文件读写器 */
static bool s_writable = false; /* 配置文件是否可写 */

CfgValue::CfgValue(const std::string& value)
{
    m_value = value;
}

bool CfgValue::isValid() const
{
    return !m_value.empty();
}

int CfgValue::toInt() const
{
    int value = 0;
    try
    {
        value = std::atoi(m_value.c_str());
    }
    catch (...)
    {
    }
    return value;
}

long long CfgValue::toLongLong() const
{
    long long value = 0;
    try
    {
        value = std::atoll(m_value.c_str());
    }
    catch (...)
    {
    }
    return value;
}

float CfgValue::toFloat() const
{
    float value = 0.0f;
    try
    {
        value = std::atof(m_value.c_str());
    }
    catch (...)
    {
    }
    return value;
}

std::string CfgValue::toString() const
{
    return m_value;
}

bool Config::init(const std::string& path, bool writable)
{
    std::lock_guard<std::mutex> locker(s_mutex);
    s_iniWriter = std::make_shared<ini::IniWriter>();
    s_iniWriter->setAllowAutoCreate();
    s_writable = writable;
    std::string errorDesc;
    if (0 == s_iniWriter->open(path + "/" + CONFIG_FILENAME, errorDesc))
    {
        if (ini::syncIni(s_iniWriter, CONFIG_FILENAME) < 2) /* 主要用于升级时同步配置文件的修改 */
        {
            return true;
        }
    }
    else
    {
        if (ini::restoreIni(s_iniWriter, CONFIG_FILENAME) < 2)
        {
            return true;
        }
    }
    return false;
}

bool Config::reload()
{
    std::lock_guard<std::mutex> locker(s_mutex);
    std::string errorDesc;
    if (0 == s_iniWriter->reload(errorDesc))
    {
        return true;
    }
    return false;
}

bool Config::restoreFactory(const std::vector<std::string>& ignoreSections, const std::vector<std::string>& ignoreItems)
{
    std::lock_guard<std::mutex> locker(s_mutex);
    if (s_writable)
    {
        if (ini::restoreIni(s_iniWriter, CONFIG_FILENAME, ignoreSections, ignoreItems) < 2)
        {
            return true;
        }
    }
    return false;
}

CfgValue Config::getValue(const std::string& key)
{
    std::string name, k;
    ini::splitSectionKey(key, name, k);
    std::lock_guard<std::mutex> locker(s_mutex);
    return CfgValue(s_iniWriter->getString(name, k));
}

bool Config::setValue(const std::string& key, int value)
{
    std::string name, k;
    ini::splitSectionKey(key, name, k);
    std::lock_guard<std::mutex> locker(s_mutex);
    if (s_writable)
    {
        if (0 == s_iniWriter->setValue(name, k, value))
        {
            return true;
        }
    }
    return false;
}

bool Config::setValue(const std::string& key, const std::string& value)
{
    std::string name, k;
    ini::splitSectionKey(key, name, k);
    std::lock_guard<std::mutex> locker(s_mutex);
    if (s_writable)
    {
        if (0 == s_iniWriter->setValue(name, k, value))
        {
            return true;
        }
    }
    return false;
}

bool Config::save()
{
    std::lock_guard<std::mutex> locker(s_mutex);
    if (s_writable)
    {
        if (0 == s_iniWriter->save())
        {
            return true;
        }
    }
    return false;
}
