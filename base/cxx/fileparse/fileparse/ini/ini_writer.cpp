#include "ini_writer.h"

#include <stdio.h>

namespace ini
{
int IniWriter::open(const std::string& filename, std::string& errorDesc)
{
    auto f = fopen(filename.c_str(), "ab+");
    if (f)
    {
        fclose(f);
    }
    return IniFile::open(filename, false, errorDesc);
}

void IniWriter::setAllowAutoCreate()
{
    m_allowAutoCreate = true;
}

bool IniWriter::save()
{
    return IniFile::save();
}

void IniWriter::clear()
{
    IniFile::clear();
}

bool IniWriter::setCommentFlags(const std::vector<std::string>& flags)
{
    return IniFile::setCommentFlags(flags);
}

bool IniWriter::removeSection(const std::string& section)
{
    return IniFile::removeSection(section);
}

int IniWriter::setSectionComment(const std::string& section, const std::string& comment)
{
    return IniFile::setSectionComment(section, comment);
}

bool IniWriter::removeKey(const std::string& section, const std::string& key)
{
    return IniFile::removeKey(section, key);
}

int IniWriter::setComment(const std::string& section, const std::string& key, const std::string& comment)
{
    return IniFile::setComment(section, key, comment);
}

int IniWriter::setValue(const std::string& section, const std::string& key, bool value)
{
    return IniFile::setValue(section, key, value ? std::string("true") : std::string("false"));
}

int IniWriter::setValue(const std::string& section, const std::string& key, int value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, unsigned int value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, unsigned long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, long long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, unsigned long long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, float value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, double value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

int IniWriter::setValue(const std::string& section, const std::string& key, std::string value)
{
    return IniFile::setValue(section, key, value);
}

bool IniWriter::isAllowAutoCreate() const
{
    return m_allowAutoCreate;
}
} // namespace ini
