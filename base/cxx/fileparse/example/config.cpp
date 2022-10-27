#include "config.h"

#include <algorithm>

std::mutex Config::m_mutex;
std::shared_ptr<ini::IniWriter> Config::m_iniWriter = nullptr;
bool Config::m_writable = false;

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
    std::lock_guard<std::mutex> locker(m_mutex);
    m_iniWriter = std::make_shared<ini::IniWriter>();
    m_iniWriter->setAllowAutoCreate();
    m_writable = writable;
    std::string errorDesc;
    if (0 == m_iniWriter->open(path + "/" + CONFIG_FILENAME, errorDesc))
    {
        if (writable)
        {
            if (0 == ini::syncIni(m_iniWriter, CONFIG_FILENAME)) /* 主要用于升级时同步配置文件的修改 */
            {
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

bool Config::reload()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    std::string errorDesc;
    if (0 == m_iniWriter->reload(errorDesc))
    {
        return true;
    }
    return false;
}

bool Config::restoreFactory()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_writable)
    {
        if (0 == ini::restoreIni(m_iniWriter, CONFIG_FILENAME))
        {
            return true;
        }
    }
    return false;
}

CfgValue Config::getValue(const std::string& key)
{
    std::string s, k;
    ini::splitSectionKey(key, s, k);
    std::lock_guard<std::mutex> locker(m_mutex);
    return CfgValue(m_iniWriter->getString(s, k));
}

void Config::setValue(const std::string& key, int value)
{
    std::string s, k;
    ini::splitSectionKey(key, s, k);
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_writable)
    {
        m_iniWriter->setValue(s, k, value);
    }
}

void Config::setValue(const std::string& key, const std::string& value)
{
    std::string s, k;
    ini::splitSectionKey(key, s, k);
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_writable)
    {
        m_iniWriter->setValue(s, k, value);
    }
}

void Config::save()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_writable)
    {
        m_iniWriter->save();
    }
}
