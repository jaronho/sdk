#pragma once

#include "inifile.h"

namespace ini
{
class IniReader : protected IniFile
{
public:
    virtual ~IniReader() = default;

    bool open(const std::string& filename, size_t lineLength = 128);

    bool reload();

    std::vector<IniSection> getDataList();

    std::vector<std::string> getCommentFlags();

    bool hasSection(const std::string& section);

    bool getSectionComment(const std::string& section, std::string& comment);

    bool hasKey(const std::string& section, const std::string& key);

    bool getComment(const std::string& section, const std::string& key, std::string& comment);

    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false);

    int getInt(const std::string& section, const std::string& key, int defaultValue = 0);

    unsigned int getUInt(const std::string& section, const std::string& key, unsigned int defaultValue = 0);

    long getLong(const std::string& section, const std::string& key, long defaultValue = 0);

    unsigned long getULong(const std::string& section, const std::string& key, unsigned long defaultValue = 0);

    long long getLongLong(const std::string& section, const std::string& key, long long defaultValue = 0);

    unsigned long long getULongLong(const std::string& section, const std::string& key, unsigned long long defaultValue = 0);

    float getFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f);

    double getDouble(const std::string& section, const std::string& key, double defaultValue = 0.0);

    std::string getString(const std::string& section, const std::string& key, std::string defaultValue = std::string());

    bool isAllowAutoCreate() override;

private:
    std::string m_filename;
    size_t m_lineLength;
};
} // namespace ini
