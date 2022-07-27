#include "inifile.h"

#include <algorithm>
#include <fstream>
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

size_t getLine(std::fstream& f, std::string& line)
{
    std::getline(f, line);
    if (line.size() >= 3 && (0xEF == line[0] && 0xBB == line[1] && 0xBF == line[2]))
    {
        line = line.substr(3); /* 跳过BOM */
    }
    return line.size();
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
    std::fstream f(filename, std::ios::in);
    if (!f.is_open())
    {
        errorDesc = "can't open file [" + filename + "]";
        return 1;
    }
    m_filename = filename;
    m_sections.clear();
    auto sectionIter = m_sections.insert(std::make_pair(std::string(), IniSection())).first;
    std::string line;
    std::string comment;
    size_t lineNumber = 0;
    while (!f.eof())
    {
        getLine(f, line);
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
                    f.close();
                    return 2;
                }
                size_t sectionNameLen = sectionTailIndex - 1;
                if (sectionNameLen <= 0) /* 节名称为空, 说明格式不对 */
                {
                    f.close();
                    errorDesc = "section name is empty, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 3;
                }
                std::string sectionName(line, 1, sectionNameLen);
                if (m_sections.end() != m_sections.find(sectionName)) /* 节名称已经存在, 说明格式不对 */
                {
                    f.close();
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
                    IniItem item;
                    item.key = key;
                    item.value = value;
                    item.comment = comment;
                    sectionIter->second.items.emplace_back(item);
                    comment.clear();
                }
                else /* 项解析出错, 说明格式不对 */
                {
                    f.close();
                    errorDesc = "format error, line[" + std::to_string(lineNumber) + "]: " + line;
                    return 5;
                }
            }
        }
        line.clear();
    }
    f.close();
    return 0;
}

bool IniFile::save()
{
    if (!m_changed)
    {
        return true;
    }
    auto f = fopen(m_filename.c_str(), "wb+");
    if (!f)
    {
        return false;
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
    fwrite(data.c_str(), 1, data.size(), f);
    fflush(f);
    fclose(f);
    m_changed = false;
    return true;
}

void IniFile::clear()
{
    m_sections.clear();
    m_changed = true;
}

std::map<std::string, IniSection> IniFile::getSections() const
{
    return m_sections;
}

std::vector<std::string> IniFile::getCommentFlags(void) const
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
    m_changed = true;
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

bool IniFile::setSectionComment(const std::string& sectionName, const std::string& comment)
{
    if (sectionName.empty() || !isComment(comment))
    {
        return false;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return false;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    sectionIter->second.comment = comment;
    m_changed = true;
    return true;
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

bool IniFile::setValue(const std::string& sectionName, const std::string& key, const std::string& value)
{
    if (key.empty())
    {
        return false;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return false;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            itemIter->value = value;
            m_changed = true;
            return true;
        }
    }
    if (isAllowAutoCreate())
    {
        IniItem item;
        item.key = key;
        item.value = value;
        sectionIter->second.items.emplace_back(item);
        m_changed = true;
        return true;
    }
    return false;
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

bool IniFile::setComment(const std::string& sectionName, const std::string& key, const std::string& comment)
{
    if (key.empty() || !isComment(comment))
    {
        return false;
    }
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() == sectionIter)
    {
        if (!isAllowAutoCreate())
        {
            return false;
        }
        IniSection section;
        section.name = sectionName;
        sectionIter = m_sections.insert(std::make_pair(sectionName, section)).first;
    }
    for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
    {
        if (key == itemIter->key)
        {
            itemIter->comment = comment;
            m_changed = true;
            return true;
        }
    }
    IniItem item;
    item.key = key;
    item.comment = comment;
    sectionIter->second.items.emplace_back(item);
    m_changed = true;
    return true;
}

bool IniFile::getExtra(const std::string& sectionName, const std::string& key, std::string& extra) const
{
    extra.clear();
    auto sectionIter = m_sections.find(sectionName);
    if (m_sections.end() != sectionIter)
    {
        for (auto itemIter = sectionIter->second.items.begin(); sectionIter->second.items.end() != itemIter; ++itemIter)
        {
            if (key == itemIter->key)
            {
                extra = itemIter->extra;
                return true;
            }
        }
    }
    return false;
}

bool IniFile::setExtra(const std::string& sectionName, const std::string& key, const std::string& extra)
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
            itemIter->extra = extra;
            return true;
        }
    }
    return false;
}

bool IniFile::isComment(const std::string& str) const
{
    for (size_t i = 0; i < m_commentFlags.size(); ++i)
    {
        const auto& commentFlag = m_commentFlags[i];
        if (str.size() < commentFlag.size())
        {
            continue;
        }
        size_t index = 0;
        for (index = 0; index < commentFlag.size(); ++index)
        {
            if (str[index] != commentFlag[index])
            {
                break;
            }
        }
        if (index == commentFlag.size())
        {
            return true;
        }
    }
    return false;
}
} // namespace ini
