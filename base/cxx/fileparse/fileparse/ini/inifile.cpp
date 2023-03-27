#include "inifile.h"

#include <algorithm>
#include <sstream>
#include <string.h>

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
    size_t len = content.size();
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
                for (size_t i = 0; i < m_commentFlags.size(); ++i)
                {
                    size_t commentHeadIndex = line.find(m_commentFlags[i]);
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
                size_t sectionTailIndex = line.find_first_of(']');
                if (std::string::npos == sectionTailIndex) /* 没有找到节结束标识, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "section format error, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 2;
                }
                size_t sectionNameLen = sectionTailIndex - 1;
                if (sectionNameLen <= 0) /* 节名称为空, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "section name is empty, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 3;
                }
                std::string sectionName(line, 1, sectionNameLen);
                if (m_sections.end() != m_sections.find(sectionName)) /* 节名称已经存在, 说明格式不对 */
                {
                    fclose(f);
                    errorDesc = "section name [" + sectionName + "] is duplicated, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 4;
                }
                IniSection section;
                section.name = sectionName;
                section.comment = comment;
                sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
                comment.clear();
            }
            else /* 项内容 */
            {
                std::string key, value;
                if (getKeyAndValue(line, key, value, '='))
                {
                    if (m_sections.end() == sectionIter)
                    {
                        sectionIter = m_sections.insert(std::make_pair(std::string(), IniSection())).first;
                    }
                    else
                    {
                        for (const auto& item : sectionIter->second.items)
                        {
                            if (key == item.key) /* 键名称已经存在, 说明格式不对 */
                            {
                                fclose(f);
                                errorDesc = "key name [" + key + "] in section [" + sectionIter->second.name + "] is duplicated, line["
                                            + std::to_string(lineNumber) + "]: " + line;
                                return 6;
                            }
                        }
                    }
                    IniItem item;
                    item.key = key;
                    item.value = value;
                    item.comment = comment;
                    sectionIter->second.items.emplace_back(item);
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

int IniFile::save()
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
    std::string data;
    for (auto sectionIter = m_sections.begin(); m_sections.end() != sectionIter; ++sectionIter)
    {
        if (m_sections.begin() != sectionIter)
        {
            data.append("\n");
        }
        if (!sectionIter->second.comment.empty())
        {
            data.append(sectionIter->second.comment).append("\n"); /* 写节注释 */
        }
        if (!sectionIter->first.empty())
        {
            data.append("[").append(sectionIter->first).append("]").append("\n"); /* 写节名称 */
        }
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (!itemIter->comment.empty())
            {
                data.append(itemIter->comment).append("\n"); /* 写项注释 */
            }
            data.append(itemIter->key).append("=").append(itemIter->value).append("\n"); /* 写项内容 */
        }
    }
    bool ret = false;
    if (data.size() == fwrite(data.c_str(), 1, data.size(), f))
    {
        if (0 == fflush(f))
        {
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

std::unordered_map<std::string, IniSection> IniFile::getSections() const
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

bool IniFile::hasSection(const std::string& sectionName) const
{
    return m_sections.end() != m_sections.find(sectionName);
}

bool IniFile::removeSection(const std::string& sectionName)
{
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        return false;
    }
    m_sections.erase(sectionIter);
    m_changed = true;
    return true;
}

bool IniFile::getSectionComment(const std::string& sectionName, std::string& comment) const
{
    comment.clear();
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        return false;
    }
    comment = sectionIter->second.comment;
    return true;
}

int IniFile::setSectionComment(const std::string& sectionName, const std::string& comment)
{
    if (sectionName.empty() || !isComment(comment))
    {
        return 1;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return 2;
        }
        if (comment.empty())
        {
            return 3;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    else if (comment == sectionIter->second.comment)
    {
        return 3;
    }
    sectionIter->second.comment = comment;
    m_changed = true;
    return 0;
}

bool IniFile::hasKey(const std::string& sectionName, const std::string& key) const
{
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                return true;
            }
        }
    }
    return false;
}

bool IniFile::removeKey(const std::string& sectionName, const std::string& key)
{
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                sectionIter->second.items.erase(itemIter);
                m_changed = true;
                return true;
            }
        }
    }
    return false;
}

bool IniFile::getValue(const std::string& sectionName, const std::string& key, std::string& value) const
{
    value.clear();
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                value = itemIter->value;
                return true;
            }
        }
    }
    return false;
}

int IniFile::setValue(const std::string& sectionName, const std::string& key, const std::string& value)
{
    if (key.empty())
    {
        return 1;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return 2;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            if (value == itemIter->value)
            {
                return 3;
            }
            itemIter->value = value;
            m_changed = true;
            return 0;
        }
    }
    if (isAllowAutoCreate())
    {
        IniItem item;
        item.key = key;
        item.value = value;
        sectionIter->second.items.emplace_back(item);
        m_changed = true;
        return 0;
    }
    return 2;
}

bool IniFile::getComment(const std::string& sectionName, const std::string& key, std::string& comment) const
{
    comment.clear();
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                comment = itemIter->comment;
                return true;
            }
        }
    }
    return false;
}

int IniFile::setComment(const std::string& sectionName, const std::string& key, const std::string& comment)
{
    if (key.empty() || !isComment(comment))
    {
        return 1;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return 2;
        }
        if (comment.empty())
        {
            return 3;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            if (comment == itemIter->comment)
            {
                return 3;
            }
            itemIter->comment = comment;
            m_changed = true;
            return 0;
        }
    }
    if (isAllowAutoCreate())
    {
        IniItem item;
        item.key = key;
        item.comment = comment;
        sectionIter->second.items.emplace_back(item);
        m_changed = true;
        return 0;
    }
    return 2;
}

bool IniFile::getExtra(const std::string& sectionName, const std::string& key, const std::string& extraName, std::string& extraValue) const
{
    extraValue.clear();
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                auto extraTter = itemIter->extraMap.find(extraName);
                if (itemIter->extraMap.end() == extraTter)
                {
                    break;
                }
                extraValue = extraTter->second;
                return true;
            }
        }
    }
    return false;
}

bool IniFile::setExtra(const std::string& sectionName, const std::string& key, const std::string& extraName, const std::string& extraValue)
{
    if (key.empty())
    {
        return false;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        return false;
    }
    for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
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
