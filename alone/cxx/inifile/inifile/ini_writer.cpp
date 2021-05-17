#include "ini_writer.h"

namespace ini
{
bool IniWriter::open(const std::string& filename, size_t lineLength)
{
#if _WIN32
    FILE* fp;
    if (0 == fopen_s(&fp, filename.c_str(), "a+"))
    {
#else
    FILE* fp = fopen(filename.c_str(), "a+");
    if (fp)
    {
#endif
        fclose(fp);
    }
    int ret = IniFile::open(filename, false, lineLength);
    return 0 == ret;
}

void IniWriter::setAllowAutoCreate()
{
    m_allowAutoCreate = true;
}

bool IniWriter::save()
{
    return IniFile::save();
}

bool IniWriter::setCommentFlags(const std::vector<std::string>& flags)
{
    return IniFile::setCommentFlags(flags);
}

bool IniWriter::removeSection(const std::string& section)
{
    return IniFile::removeSection(section);
}

bool IniWriter::setSectionComment(const std::string& section, const std::string& comment)
{
    return IniFile::setSectionComment(section, comment);
}

bool IniWriter::removeKey(const std::string& section, const std::string& key)
{
    return IniFile::removeKey(section, key);
}

bool IniWriter::setComment(const std::string& section, const std::string& key, const std::string& comment)
{
    return IniFile::setComment(section, key, comment);
}

bool IniWriter::setValue(const std::string& section, const std::string& key, bool value)
{
    return IniFile::setValue(section, key, value ? std::string("true") : std::string("false"));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, int value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, unsigned int value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, unsigned long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, long long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, unsigned long long value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, float value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, double value)
{
    return IniFile::setValue(section, key, std::to_string(value));
}

bool IniWriter::setValue(const std::string& section, const std::string& key, std::string value)
{
    return IniFile::setValue(section, key, value);
}

bool IniWriter::isAllowAutoCreate()
{
    return m_allowAutoCreate;
}
} // namespace ini
