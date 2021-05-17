#include "inifile.h"

#include <algorithm>
#include <sstream>
#include <string.h>

namespace ini
{
void trimLeft(std::string& str, char c)
{
    size_t len = str.size();
    size_t i = 0;
    while (str[i] == c && '\0' != str[i])
    {
        ++i;
    }
    if (0 != i)
    {
        str = std::string(str, i, len - i);
    }
}

void trimRight(std::string& str, char c)
{
    int len = static_cast<int>(str.size());
    int i = 0;
    for (i = len - 1; i >= 0; --i)
    {
        if (c != str[i])
        {
            break;
        }
    }
    str = std::string(str, 0, static_cast<size_t>(i) + 1);
}

void trimLeftRightSpace(std::string& str)
{
    int len = static_cast<int>(str.size());
    int i = 0;
    while (isspace(str[i]) && '\0' != str[i])
    {
        ++i;
    }
    if (0 != i)
    {
        str = std::string(str, i, static_cast<size_t>(len) - i);
    }
    len = str.size();
    for (i = len - 1; i >= 0; --i)
    {
        if (!isspace(str[i]))
        {
            break;
        }
    }
    str = std::string(str, 0, static_cast<size_t>(i) + 1);
}

size_t getLine(FILE* fp, std::string& line, size_t lineSize)
{
    size_t bufSize = lineSize * sizeof(char);
    char* buf = (char*)malloc(bufSize);
    if (!buf)
    {
        return 0;
    }
    memset(buf, 0, bufSize);
    size_t totalSize = bufSize;
    char* p = buf;
    while (fgets(p, bufSize, fp))
    {
        size_t plen = strlen(p);
        if (plen > 0 && '\n' != p[plen - 1] && !feof(fp))
        {
            totalSize = strlen(buf) + bufSize;
            char* pbuf = (char*)realloc(buf, totalSize);
            if (!pbuf)
            {
                free(buf);
                return 0;
            }
            buf = pbuf;
            p = buf + strlen(buf);
            continue;
        }
        else
        {
            break;
        }
    }
    char* tmp = buf;
    if (strlen(tmp) >= 3 && (0xEF == tmp[0] && 0xBB == tmp[1] && 0xBF == tmp[2]))
    {
        tmp = tmp + 3; /* 跳过BOM */
    }
    line = tmp;
    free(buf);
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
    m_commentFlags.push_back("#");
    m_commentFlags.push_back(";");
}

IniFile::~IniFile()
{
    save();
}

int IniFile::open(const std::string& filename, bool allowTailComment, size_t lineLength)
{
#if _WIN32
    FILE* fp;
    if (0 != fopen_s(&fp, filename.c_str(), "r"))
    {
#else
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp)
    {
#endif
        return 1;
    }
    m_filename = filename;
    auto section = std::make_shared<IniSection>();
    m_sections.clear();
    m_sections.insert(std::make_pair(std::string(), section));
    std::string line;
    std::string comment;
    while (getLine(fp, line, lineLength) > 0)
    {
        trimLeft(line, ' ');
        trimRight(line, '\r');
        trimRight(line, '\n');
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
                    fclose(fp);
                    return 2;
                }
                size_t sectionNameLen = sectionTailIndex - 1;
                if (sectionNameLen <= 0) /* 节名称为空, 说明格式不对 */
                {
                    fclose(fp);
                    return 3;
                }
                std::string sectionName(line, 1, sectionNameLen);
                if (m_sections.end() != m_sections.find(sectionName)) /* 节名称已经存在, 说明格式不对 */
                {
                    fclose(fp);
                    return 4;
                }
                section = std::make_shared<IniSection>();
                section->name = sectionName;
                section->comment = comment;
                m_sections.insert(std::make_pair(sectionName, section));
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
                    section->items.push_back(item);
                    comment.clear();
                }
                else /* 项解析出错, 说明格式不对 */
                {
                    fclose(fp);
                    return 5;
                }
            }
        }
    }
    fclose(fp);
    return 0;
}

bool IniFile::save()
{
    if (m_saved)
    {
        return true;
    }
#if _WIN32
    FILE* fp;
    if (0 != fopen_s(&fp, m_filename.c_str(), "w"))
    {
#else
    FILE* fp = fopen(m_filename.c_str(), "w");
    if (!fp)
    {
#endif
        return false;
    }
    std::string data;
    for (auto iter = m_sections.begin(); m_sections.end() != iter; ++iter)
    {
        const std::string& sectionName = iter->first;
        const auto& section = iter->second;
        if (!section->comment.empty())
        {
            data.append(section->comment).append("\n"); /* 写节注释 */
        }
        if (!sectionName.empty())
        {
            data.append("[").append(sectionName).append("]").append("\n"); /* 写节名称 */
        }
        for (auto item = section->items.begin(); section->items.end() != item; ++item)
        {
            if (!item->comment.empty())
            {
                data.append(item->comment).append("\n"); /* 写项注释 */
            }
            data.append(item->key).append("=").append(item->value).append("\n"); /* 写项内容 */
        }
    }
    fwrite(data.c_str(), 1, data.size(), fp);
    fflush(fp);
    fclose(fp);
    m_saved = true;
    return true;
}

std::vector<IniSection> IniFile::getDataList()
{
    std::vector<IniSection> dataList;
    for (auto iter = m_sections.begin(); m_sections.end() != iter; ++iter)
    {
        dataList.push_back(*(iter->second.get()));
    }
    return dataList;
}

std::vector<std::string> IniFile::getCommentFlags(void)
{
    return m_commentFlags;
}

bool IniFile::setCommentFlags(const std::vector<std::string>& flags)
{
    if (!flags.empty())
    {
        m_commentFlags = flags;
        m_saved = false;
        return true;
    }
    return false;
}

bool IniFile::hasSection(const std::string& sectionName)
{
    return m_sections.end() != m_sections.find(sectionName);
}

bool IniFile::removeSection(const std::string& sectionName)
{
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        m_sections.erase(iter);
        m_saved = false;
        return true;
    }
    return false;
}

bool IniFile::getSectionComment(const std::string& sectionName, std::string& comment)
{
    comment.clear();
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        comment = iter->second->comment;
        return true;
    }
    return false;
}

bool IniFile::setSectionComment(const std::string& sectionName, const std::string& comment)
{
    if (sectionName.empty() || !isComment(comment))
    {
        return false;
    }
    std::shared_ptr<IniSection> section;
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() == iter)
    {
        if (isAllowAutoCreate())
        {
            section = std::make_shared<IniSection>();
            section->name = sectionName;
            m_sections.insert(std::make_pair(sectionName, section));
        }
        else
        {
            return false;
        }
    }
    else
    {
        section = iter->second;
    }
    section->comment = comment;
    m_saved = false;
    return true;
}

bool IniFile::hasKey(const std::string& sectionName, const std::string& key)
{
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        auto section = iter->second;
        for (auto item = section->items.begin(); section->items.end() != item; ++item)
        {
            if (key == item->key)
            {
                return true;
            }
        }
    }
    return false;
}

bool IniFile::removeKey(const std::string& sectionName, const std::string& key)
{
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        auto section = iter->second;
        for (auto item = section->items.begin(); section->items.end() != item; ++item)
        {
            if (key == item->key)
            {
                section->items.erase(item);
                m_saved = false;
                return true;
            }
        }
    }
    return false;
}

bool IniFile::getValue(const std::string& sectionName, const std::string& key, std::string& value)
{
    value.clear();
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        auto section = iter->second;
        for (auto item = section->items.begin(); section->items.end() != item; ++item)
        {
            if (key == item->key)
            {
                value = item->value;
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
    std::shared_ptr<IniSection> section;
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() == iter)
    {
        if (isAllowAutoCreate())
        {
            section = std::make_shared<IniSection>();
            section->name = sectionName;
            m_sections.insert(std::make_pair(sectionName, section));
        }
        else
        {
            return false;
        }
    }
    else
    {
        section = iter->second;
    }
    for (auto item = section->items.begin(); section->items.end() != item; ++item)
    {
        if (key == item->key)
        {
            item->value = value;
            m_saved = false;
            return true;
        }
    }
    if (isAllowAutoCreate())
    {
        IniItem item;
        item.key = key;
        item.value = value;
        section->items.push_back(item);
        m_saved = false;
        return true;
    }
    return false;
}

bool IniFile::getComment(const std::string& sectionName, const std::string& key, std::string& comment)
{
    comment.clear();
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() != iter)
    {
        auto section = iter->second;
        for (auto item = section->items.begin(); section->items.end() != item; ++item)
        {
            if (key == item->key)
            {
                comment = item->comment;
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
    std::shared_ptr<IniSection> section;
    auto iter = m_sections.find(sectionName);
    if (m_sections.end() == iter)
    {
        if (isAllowAutoCreate())
        {
            section = std::make_shared<IniSection>();
            section->name = sectionName;
            m_sections.insert(std::make_pair(sectionName, section));
        }
        else
        {
            return false;
        }
    }
    else
    {
        section = iter->second;
    }
    for (auto item = section->items.begin(); section->items.end() != item; ++item)
    {
        if (key == item->key)
        {
            item->comment = comment;
            m_saved = false;
            return true;
        }
    }
    IniItem item;
    item.key = key;
    item.comment = comment;
    section->items.push_back(item);
    m_saved = false;
    return true;
}

bool IniFile::isComment(const std::string& str)
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
