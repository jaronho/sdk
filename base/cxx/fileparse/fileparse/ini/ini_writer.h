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

    int setSectionComment(const std::string& section, const std::string& comment);

    bool removeKey(const std::string& section, const std::string& key);

    int setComment(const std::string& section, const std::string& key, const std::string& comment);

    int setValue(const std::string& section, const std::string& key, bool value);

    int setValue(const std::string& section, const std::string& key, int value);

    int setValue(const std::string& section, const std::string& key, unsigned int value);

    int setValue(const std::string& section, const std::string& key, long value);

    int setValue(const std::string& section, const std::string& key, unsigned long value);

    int setValue(const std::string& section, const std::string& key, long long value);

    int setValue(const std::string& section, const std::string& key, unsigned long long value);

    int setValue(const std::string& section, const std::string& key, float value);

    int setValue(const std::string& section, const std::string& key, double value);

    int setValue(const std::string& section, const std::string& key, std::string value);

    bool isAllowAutoCreate() const override;

private:
    bool m_allowAutoCreate = false;
};
} // namespace ini
