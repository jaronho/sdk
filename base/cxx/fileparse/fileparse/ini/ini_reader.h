#pragma once
#include "inifile.h"

namespace ini
{
class IniReader : protected IniFile
{
public:
    virtual ~IniReader() = default;

    int open(const std::string& filename, std::string& errorDesc);

    int reload(std::string& errorDesc);

    std::map<std::string, IniSection> getSections() const;

    std::vector<std::string> getCommentFlags() const;

    bool hasSection(const std::string& section) const;

    bool getSectionComment(const std::string& section, std::string& comment) const;

    bool hasKey(const std::string& section, const std::string& key) const;

    bool getComment(const std::string& section, const std::string& key, std::string& comment) const;

    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false) const;

    int getInt(const std::string& section, const std::string& key, int defaultValue = 0) const;

    unsigned int getUInt(const std::string& section, const std::string& key, unsigned int defaultValue = 0) const;

    long getLong(const std::string& section, const std::string& key, long defaultValue = 0) const;

    unsigned long getULong(const std::string& section, const std::string& key, unsigned long defaultValue = 0) const;

    long long getLongLong(const std::string& section, const std::string& key, long long defaultValue = 0) const;

    unsigned long long getULongLong(const std::string& section, const std::string& key, unsigned long long defaultValue = 0) const;

    float getFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f) const;

    double getDouble(const std::string& section, const std::string& key, double defaultValue = 0.0) const;

    std::string getString(const std::string& section, const std::string& key, std::string defaultValue = std::string()) const;

    bool isAllowAutoCreate() const override;

private:
    std::string m_filename;
};
} // namespace ini
