#include "inifile.h"

#include <algorithm>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace ini
{
void trimLeft(std::string& str, char c)
{
    if (str.empty())
    {
        return;
    }
    str.erase(0, str.find_first_not_of(c));
}

void trimRight(std::string& str, char c)
{
    if (str.empty())
    {
        return;
    }
    str.erase(str.find_last_not_of(c) + 1);
}

void trimLeftRightSpace(std::string& str)
{
    trimLeft(str, ' ');
    trimRight(str, ' ');
}

void trimLeftRightCRLF(std::string& str)
{
    /* 左边顺序为: 先除\r, 再除\n */
    trimLeft(str, '\r');
    trimLeft(str, '\n');
    /* 右边顺序为: 先除\n, 再除\r */
    trimRight(str, '\n');
    trimRight(str, '\r');
}

std::string getLine(FILE* f)
{
    std::string line;
    if (f)
    {
        char ch;
        while (!feof(f) && '\n' != (ch = fgetc(f)))
        {
            line.push_back(ch);
        }
        /* BOM字符检测 */
        if (line.size() >= 3 && (0xEF == (unsigned char)line[0] && 0xBB == (unsigned char)line[1] && 0xBF == (unsigned char)line[2]))
        {
            line = line.substr(3);
        }
        /* 非显示字符检测 */
        long long lastIndex = line.size() - 1;
        if (lastIndex >= 0 && ('\r' == line[lastIndex] || 0xFF == (unsigned char)line[lastIndex]))
        {
            line.erase(lastIndex);
        }
    }
    return line;
}

bool getKeyAndValue(const std::string& content, std::string& key, std::string& value, char c)
{
    auto len = content.size();
    size_t i = 0;
    while (i < len && c != content[i])
    {
        ++i;
    }
    if (i >= 0 && i < len)
    {
        key = std::string(content.c_str(), i);
        value = std::string(content.c_str() + i + 1, len - i - 1);
        trimLeftRightSpace(key);
        trimLeftRightSpace(value);
        return true;
    }
    return false;
}

IniFile::IniFile()
{
    m_commentFlags.emplace_back("#");
    m_commentFlags.emplace_back(";");
}

IniFile::~IniFile()
{
    save();
}

int IniFile::open(const std::string& filename, bool allowTailComment, std::string& errorDesc)
{
    errorDesc.clear();
    auto f = fopen(filename.c_str(), "rb");
    if (!f)
    {
        errorDesc = "can't open file [" + filename + "]";
        return 1;
    }
    m_filename = filename;
    m_sections.clear();
    auto sectionIter = m_sections.begin();
    std::string comment;
    size_t lineNumber = 0;
    while (!feof(f))
    {
        auto line = getLine(f);
        ++lineNumber;
        trimLeftRightSpace(line);
        trimLeftRightCRLF(line);
        if (line.empty())
        {
            continue;
        }
        if (isComment(line)) /* 为注释行 */
        {
            if (comment.empty())
            {
                comment = line;
            }
            else
            {
                comment.append("\n").append(line);
            }
        }
        else
        {
            if (allowTailComment) /* 允许在行尾注释 */
            {
                /*
                 * 针对"value=1 #测试"这种后面有注释的语句, 再次保存时将会变为:
                 * #测试
                 * value=1
                 */
                for (size_t i = 0, len = m_commentFlags.size(); i < len; ++i)
                {
                    auto commentHeadIndex = line.find(m_commentFlags[i]);
                    if (std::string::npos != commentHeadIndex)
                    {
                        comment = line.substr(commentHeadIndex, line.size() - commentHeadIndex);
                        line = line.substr(0, commentHeadIndex);
                        break;
                    }
                }
            }
            trimLeftRightSpace(line);
            if (line.empty())
            {
                continue;
            }
            if ('[' == line[0]) /* 找到节起始标识 */
            {
                auto sectionTailIndex = line.find_first_of(']');
                if (std::string::npos == sectionTailIndex) /* 没有找到节结束标识, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "section format error, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 2;
                }
                auto sectionNameLen = sectionTailIndex - 1;
                if (sectionNameLen <= 0) /* 节名称为空, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "section is empty, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 3;
                }
                std::string name(line, 1, sectionNameLen);
                for (const auto& section : m_sections)
                {
                    if (name == section.name) /* 节名称已经存在, 说明格式不对 */
                    {
                        fclose(f);
                        errorDesc = "section [" + name + "] is duplicated, line[" + std::to_string(lineNumber) + "]: " + line;
                        return 4;
                    }
                }
                IniSection section;
                section.name = name;
                section.comment = comment;
                sectionIter = m_sections.insert(m_sections.end(), section);
                comment.clear();
            }
            else /* 项内容 */
            {
                std::string key, value;
                if (getKeyAndValue(line, key, value, '='))
                {
                    if (m_sections.end() == sectionIter)
                    {
                        sectionIter = m_sections.insert(m_sections.end(), IniSection());
                    }
                    else
                    {
                        for (const auto& item : sectionIter->items)
                        {
                            if (key == item.key) /* 项已经存在, 说明格式不对 */
                            {
                                fclose(f);
                                errorDesc = "key [" + key + "] in section [" + sectionIter->name + "] is duplicated, line["
                                            + std::to_string(lineNumber) + "]: " + line;
                                return 6;
                            }
                        }
                    }
                    IniItem item;
                    item.key = key;
                    item.value = value;
                    item.comment = comment;
                    sectionIter->items.emplace_back(item);
                    comment.clear();
                }
                else /* 项解析出错, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "format error, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 5;
                }
            }
        }
    }
    fclose(f);
    return 0;
}

int IniFile::save(int sortType)
{
    if (!m_changed)
    {
        return 1;
    }
    auto f = fopen(m_filename.c_str(), "wb+");
    if (!f)
    {
        return 2;
    }
    sortType = (1 == sortType || 2 == sortType) ? sortType : 0;
    std::string data;
    if (1 == sortType || 2 == sortType)
    {
        std::sort(m_sections.begin(), m_sections.end(), [&](const IniSection& a, const IniSection& b) {
            if (a.name.empty())
            {
                return true;
            }
            else if (b.name.empty())
            {
                return false;
            }
            return (1 == sortType ? (a.name < b.name) : (a.name > b.name));
        });
    }
    for (auto& section : m_sections)
    {
        if (!data.empty())
        {
            data.append("\n");
        }
        if (!section.comment.empty())
        {
            data.append(section.comment).append("\n"); /* 写节注释 */
        }
        if (!section.name.empty())
        {
            data.append("[").append(section.name).append("]").append("\n"); /* 写节名称 */
        }
        if (1 == sortType || 2 == sortType)
        {
            std::sort(section.items.begin(), section.items.end(),
                      [&](const IniItem& a, const IniItem& b) { return (1 == sortType ? (a.key < b.key) : (a.key > b.key)); });
        }
        for (const auto& item : section.items)
        {
            if (!item.comment.empty())
            {
                data.append(item.comment).append("\n"); /* 写项注释 */
            }
            data.append(item.key).append("=").append(item.value).append("\n"); /* 写项内容 */
        }
    }
    bool ret = false;
    if (data.size() == fwrite(data.c_str(), 1, data.size(), f))
    {
        if (0 == fflush(f))
        {
#ifndef _WIN32
            fsync(fileno(f)); /* 同步到磁盘 */
#endif
            ret = true;
        }
    }
    if (0 != fclose(f))
    {
        ret = false;
    }
    m_changed = false;
    return (ret ? 0 : 3);
}

void IniFile::clear()
{
    m_sections.clear();
    m_changed = true;
}

std::vector<IniSection> IniFile::getSections() const
{
    return m_sections;
}

std::vector<std::string> IniFile::getCommentFlags() const
{
    return m_commentFlags;
}

bool IniFile::setCommentFlags(const std::vector<std::string>& flags)
{
    if (flags.empty())
    {
        return false;
    }
    m_commentFlags = flags;
    return true;
}

bool IniFile::hasSection(const std::string& name) const
{
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            return true;
        }
    }
    return false;
}

bool IniFile::removeSection(const std::string& name)
{
    for (auto iter = m_sections.begin(); m_sections.end() != iter; ++iter)
    {
        if (name == iter->name)
        {
            m_sections.erase(iter);
            m_changed = true;
            return true;
        }
    }
    return false;
}

bool IniFile::getSectionComment(const std::string& name, std::string& comment) const
{
    comment.clear();
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            comment = section.comment;
            return true;
        }
    }
    return false;
}

int IniFile::setSectionComment(const std::string& name, const std::string& comment)
{
    if (name.empty() || !isComment(comment))
    {
        return 2;
    }
    auto iter = m_sections.begin();
    for (; m_sections.end() != iter; ++iter)
    {
        if (name == iter->name)
        {
            break;
        }
    }
    if (m_sections.end() == iter)
    {
        if (!isAllowAutoCreate())
        {
            return 3;
        }
        if (comment.empty())
        {
            return 1;
        }
        IniSection section;
        section.name = name;
        iter = m_sections.insert(m_sections.end(), section);
    }
    else if (comment == iter->comment)
    {
        return 1;
    }
    iter->comment = comment;
    m_changed = true;
    return 0;
}

bool IniFile::hasItem(const std::string& name, const std::string& key) const
{
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            for (const auto& item : section.items)
            {
                if (key == item.key)
                {
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool IniFile::removeItem(const std::string& name, const std::string& key)
{
    for (auto& section : m_sections)
    {
        if (name == section.name)
        {
            for (auto iter = section.items.begin(); section.items.end() != iter; ++iter)
            {
                if (key == iter->key)
                {
                    section.items.erase(iter);
                    m_changed = true;
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool IniFile::getValue(const std::string& name, const std::string& key, std::string& value) const
{
    value.clear();
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            for (const auto& item : section.items)
            {
                if (key == item.key)
                {
                    value = item.value;
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

int IniFile::setValue(const std::string& name, const std::string& key, const std::string& value)
{
    if (key.empty())
    {
        return 2;
    }
    auto sectionIter = m_sections.begin();
    for (; m_sections.end() != sectionIter; ++sectionIter)
    {
        if (name == sectionIter->name)
        {
            break;
        }
    }
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return 3;
        }
        IniSection section;
        section.name = name;
        sectionIter = m_sections.insert(m_sections.end(), section);
    }
    auto itemIter = sectionIter->items.begin();
    for (; sectionIter->items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            if (value == itemIter->value)
            {
                return 1;
            }
            itemIter->value = value;
            m_changed = true;
            return 0;
        }
    }
    if (!isAllowAutoCreate())
    {
        return 3;
    }
    IniItem item;
    item.key = key;
    item.value = value;
    sectionIter->items.emplace_back(item);
    m_changed = true;
    return 0;
}

bool IniFile::getComment(const std::string& name, const std::string& key, std::string& comment) const
{
    comment.clear();
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            for (const auto& item : section.items)
            {
                if (key == item.key)
                {
                    comment = item.comment;
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

int IniFile::setComment(const std::string& name, const std::string& key, const std::string& comment)
{
    if (key.empty() || !isComment(comment))
    {
        return 2;
    }
    auto sectionIter = m_sections.begin();
    for (; m_sections.end() != sectionIter; ++sectionIter)
    {
        if (name == sectionIter->name)
        {
            break;
        }
    }
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return 3;
        }
        if (comment.empty())
        {
            return 1;
        }
        IniSection section;
        section.name = name;
        sectionIter = m_sections.insert(m_sections.end(), section);
    }
    auto itemIter = sectionIter->items.begin();
    for (; sectionIter->items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            if (comment == itemIter->comment)
            {
                return 1;
            }
            itemIter->comment = comment;
            m_changed = true;
            return 0;
        }
    }
    if (!isAllowAutoCreate())
    {
        return 3;
    }
    IniItem item;
    item.key = key;
    item.comment = comment;
    sectionIter->items.emplace_back(item);
    m_changed = true;
    return 0;
}

bool IniFile::getExtra(const std::string& name, const std::string& key, const std::string& extraName, std::string& extraValue) const
{
    extraValue.clear();
    for (const auto& section : m_sections)
    {
        if (name == section.name)
        {
            for (const auto& item : section.items)
            {
                if (key == item.key)
                {
                    auto iter = item.extraMap.find(extraName);
                    if (item.extraMap.end() != iter)
                    {
                        extraValue = iter->second;
                        return true;
                    }
                    break;
                }
            }
            break;
        }
    }
    return false;
}

bool IniFile::setExtra(const std::string& name, const std::string& key, const std::string& extraName, const std::string& extraValue)
{
    if (key.empty())
    {
        return false;
    }
    auto sectionIter = m_sections.begin();
    for (; m_sections.end() != sectionIter; ++sectionIter)
    {
        if (name == sectionIter->name)
        {
            break;
        }
    }
    if (m_sections.end() == sectionIter)
    {
        return false;
    }
    auto itemIter = sectionIter->items.begin();
    for (; sectionIter->items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            itemIter->extraMap[extraName] = extraValue;
            return true;
        }
    }
    return false;
}

bool IniFile::isComment(const std::string& str) const
{
    if (str.empty()) /* 空注释 */
    {
        return true;
    }
    for (const auto& commentFlag : m_commentFlags)
    {
        if (0 == str.find(commentFlag)) /* 找到注释标识头 */
        {
            return true;
        }
    }
    return false;
}
} // namespace ini
