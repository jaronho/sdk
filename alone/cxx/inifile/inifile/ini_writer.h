#pragma once

#include "ini_reader.h"

namespace ini
{
class IniWriter final : public IniReader
{
public:
    virtual ~IniWriter() = default;

    bool open(const std::string& filename, size_t lineLength = 128);

    void setAllowAutoCreate();

    bool save();

    bool setCommentFlags(const std::vector<std::string>& flags);

    bool removeSection(const std::string& section);

    bool setSectionComment(const std::string& section, const std::string& comment);

    bool removeKey(const std::string& section, const std::string& key);

    bool setComment(const std::string& section, const std::string& key, const std::string& comment);

    bool setBool(const std::string& section, const std::string& key, bool value);

    bool setInt(const std::string& section, const std::string& key, int value);

    bool setUInt(const std::string& section, const std::string& key, unsigned int value);

    bool setLong(const std::string& section, const std::string& key, long value);

    bool setULong(const std::string& section, const std::string& key, unsigned long value);

    bool setLongLong(const std::string& section, const std::string& key, long long value);

    bool setULongLong(const std::string& section, const std::string& key, unsigned long long value);

    bool setFloat(const std::string& section, const std::string& key, float value);

    bool setDouble(const std::string& section, const std::string& key, double value);

    bool setString(const std::string& section, const std::string& key, std::string value);

protected:
    bool isAllowAutoCreate() override;

private:
    bool m_allowAutoCreate = false;
};
} // namespace ini
