#pragma once

#include "ini_reader.h"

namespace ini
{
class IniWriter final : public IniReader
{
public:
    virtual ~IniWriter() = default;

    int open(const std::string& filename, std::string& errorDesc);

    void setAllowAutoCreate();

    bool save();

    void clear();

    bool setCommentFlags(const std::vector<std::string>& flags);

    bool removeSection(const std::string& section);

    bool setSectionComment(const std::string& section, const std::string& comment);

    bool removeKey(const std::string& section, const std::string& key);

    bool setComment(const std::string& section, const std::string& key, const std::string& comment);

    bool setValue(const std::string& section, const std::string& key, bool value);

    bool setValue(const std::string& section, const std::string& key, int value);

    bool setValue(const std::string& section, const std::string& key, unsigned int value);

    bool setValue(const std::string& section, const std::string& key, long value);

    bool setValue(const std::string& section, const std::string& key, unsigned long value);

    bool setValue(const std::string& section, const std::string& key, long long value);

    bool setValue(const std::string& section, const std::string& key, unsigned long long value);

    bool setValue(const std::string& section, const std::string& key, float value);

    bool setValue(const std::string& section, const std::string& key, double value);

    bool setValue(const std::string& section, const std::string& key, std::string value);

    bool isAllowAutoCreate() override;

private:
    bool m_allowAutoCreate = false;
};
} // namespace ini