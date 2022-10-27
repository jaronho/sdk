#include "strtool.h"

#include <algorithm>
#include <string.h>

namespace utility
{
void StrTool::trimLeft(std::string& str, char c)
{
    if (str.empty())
    {
        return;
    }
    str.erase(0, str.find_first_not_of(c));
}

void StrTool::trimRight(std::string& str, char c)
{
    if (str.empty())
    {
        return;
    }
    str.erase(str.find_last_not_of(c) + 1);
}

void StrTool::trimLeftRight(std::string& str, char c)
{
    trimLeft(str, c);
    trimRight(str, c);
}

void StrTool::trimDuplicate(std::string& str, char c)
{
    std::string tmp;
    size_t sameCount = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (c == str[i])
        {
            if (++sameCount > 1)
            {
                continue;
            }
        }
        else
        {
            sameCount = 0;
        }
        tmp.push_back(str[i]);
    }
    str = tmp;
}

std::string StrTool::toLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

std::string StrTool::toUpper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
    return str;
}

std::string StrTool::replace(std::string str, const std::string& rep, const std::string& dest)
{
    if (str.empty() || rep.empty())
    {
        return str;
    }
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(rep, pos)))
    {
        str.replace(pos, rep.size(), dest);
        pos += dest.size();
    }
    return str;
}

std::vector<std::string> StrTool::split(const std::string& str, const std::string& sep)
{
    std::vector<std::string> strList;
    if (str.empty() || sep.empty())
    {
        return strList;
    }
    std::string::size_type pos;
    for (size_t i = 0; i < str.size(); ++i)
    {
        pos = str.find(sep, i);
        if (std::string::npos == pos)
        {
            pos = str.size();
        }
        strList.emplace_back(str.substr(i, pos - i));
        i = pos + sep.size() - 1;
    }
    return strList;
}

std::vector<std::string> StrTool::split(const std::string& str, int sepNum)
{
    std::vector<std::string> strList;
    if (str.empty())
    {
        return strList;
    }
    if (sepNum > 0)
    {
        std::string sepStr;
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (sepStr.size() == sepNum)
            {
                strList.emplace_back(sepStr);
                sepStr.clear();
            }
            sepStr.push_back(str[i]);
        }
        if (!sepStr.empty())
        {
            strList.emplace_back(sepStr);
        }
    }
    else
    {
        strList.emplace_back(str);
    }
    return strList;
}

std::string StrTool::join(const std::vector<std::string>& strList, const std::string& sep, size_t count)
{
    if (0 == count || count > strList.size())
    {
        count = strList.size();
    }
    std::string str;
    for (size_t i = 0; i < count; ++i)
    {
        str += (i > 0 ? sep : "") + strList[i];
    }
    return str;
}

bool StrTool::equal(std::string str1, std::string str2, bool caseSensitive)
{
    if (!caseSensitive)
    {
        std::transform(str1.begin(), str1.end(), str1.begin(), tolower);
        std::transform(str2.begin(), str2.end(), str2.begin(), tolower);
    }
    if (0 == str1.compare(str2))
    {
        return true;
    }
    return false;
}

size_t StrTool::indexOf(std::string str, std::string pattern, size_t offset, bool caseSensitive)
{
    if (pattern.empty())
    {
        return std::string::npos;
    }
    if (pattern.size() > str.size())
    {
        return std::string::npos;
    }
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(pattern.begin(), pattern.end(), pattern.begin(), tolower);
    }
    if (std::string::npos == offset)
    {
        offset = 0;
    }
    return str.find(pattern, offset);
}

bool StrTool::contains(std::string str, std::string pattern, bool caseSensitive)
{
    if (pattern.empty())
    {
        return true;
    }
    if (pattern.size() > str.size())
    {
        return false;
    }
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(pattern.begin(), pattern.end(), pattern.begin(), tolower);
    }
    if (std::string::npos == str.find(pattern))
    {
        return false;
    }
    return true;
}

bool StrTool::isBeginWith(std::string str, std::string beg, bool caseSensitive)
{
    if (beg.empty())
    {
        return true;
    }
    if (beg.size() > str.size())
    {
        return false;
    }
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(beg.begin(), beg.end(), beg.begin(), tolower);
    }
    if (0 == str.compare(0, beg.size(), beg))
    {
        return true;
    }
    return false;
}

bool StrTool::isEndWith(std::string str, std::string end, bool caseSensitive)
{
    if (end.empty())
    {
        return true;
    }
    if (end.size() > str.size())
    {
        return false;
    }
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(end.begin(), end.end(), end.begin(), tolower);
    }
    if (0 == str.compare(str.size() - end.size(), end.size(), end))
    {
        return true;
    }
    return false;
}

size_t StrTool::findCount(std::string str, std::string pattern, bool caseSensitive, bool overlap)
{
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(pattern.begin(), pattern.end(), pattern.begin(), tolower);
    }
    size_t count = 0;
    for (size_t i = 0; std::string::npos != (i = str.find(pattern, i));)
    {
        i += (overlap ? 1 : pattern.size());
        ++count;
    }
    return count;
}

std::string StrTool::toHex(const char* bytes, unsigned int byteCount, bool upper, const std::string& sep)
{
    std::string hexStr;
    if (!bytes || 0 == byteCount)
    {
        return hexStr;
    }
    char tmp[3] = {0};
    for (size_t i = 0; i < byteCount; ++i)
    {
        memset(tmp, 0, sizeof(tmp));
#ifdef _WIN32
        sprintf_s(tmp, sizeof(tmp), upper ? "%02X" : "%02x", (unsigned char)bytes[i]);
#else
        sprintf(tmp, upper ? "%02X" : "%02x", (unsigned char)bytes[i]);
#endif
        if (i > 0 && !sep.empty())
        {
            hexStr += sep;
        }
        hexStr += tmp;
    }
    return hexStr;
}

std::vector<char> StrTool::fromHex(const std::string& hexStr, const std::string& sep)
{
    std::vector<char> bytes;
    if (hexStr.empty())
    {
        return bytes;
    }
    if (sep.empty()) /* 无分隔符 */
    {
        if (0 == hexStr.size() % 2) /* 必须保证16进制字符串长度为偶数 */
        {
            bytes.resize(hexStr.size() / 2);
            unsigned char highByte, lowByte;
            for (size_t i = 0; i < hexStr.size(); i += 2)
            {
                highByte = toupper(hexStr[i]);
                highByte -= (highByte > 0x39) ? 0x37 : 0x30;
                lowByte = toupper(hexStr[i + 1]);
                lowByte -= (lowByte > 0x39) ? 0x37 : 0x30;
                bytes[i / 2] = (highByte << 4) | lowByte;
            }
        }
    }
    else /* 有分隔符 */
    {
        unsigned char highByte, lowByte;
        for (size_t i = 0; i < hexStr.size(); ++i)
        {
            auto pos = hexStr.find(sep, i);
            if (std::string::npos == pos)
            {
                pos = hexStr.size();
            }
            auto hex = hexStr.substr(i, pos - i);
            if ((3 == hex.size() || 4 == hex.size()) && 48 == hex[0] && (88 == hex[1] || 120 == hex[1])) /* 0X或0x开头 */
            {
                hex = hex.substr(2, hex.size() - 2);
            }
            else if (1 != hex.size() && 2 != hex.size())
            {
                return bytes;
            }
            if (2 != hex.size())
            {
                hex.insert(0, 1, '0');
            }
            highByte = toupper(hex[0]);
            highByte -= (highByte > 0x39) ? 0x37 : 0x30;
            lowByte = toupper(hex[1]);
            lowByte -= (lowByte > 0x39) ? 0x37 : 0x30;
            bytes.emplace_back((highByte << 4) | lowByte);
            i = pos + sep.size() - 1;
        }
    }
    return bytes;
}

std::string StrTool::fillPlace(const std::string& str, char c, size_t length, bool leftFlag)
{
    if (str.size() >= length)
    {
        return str;
    }
    auto count = length - str.size();
    if (leftFlag)
    {
        return (std::string(count, c) + str);
    }
    return (str + std::string(count, c));
}

char** StrTool::convertToArgv(const std::vector<std::string>& vec, int& argc)
{
    char** argv = NULL;
    argc = vec.size();
    if (argc > 0)
    {
        argv = (char**)malloc((argc + (size_t)1) * sizeof(char*)); /* 注意: 要多分配一个单元空间 */
        if (argv)
        {
            for (int i = 0; i < argc; ++i)
            {
                const auto& str = vec[i];
                argv[i] = (char*)malloc((str.size() + (size_t)1) * sizeof(char));
                if (argv[i])
                {
                    memcpy(argv[i], str.c_str(), str.size());
                    argv[i][str.size()] = '\0';
                }
            }
            argv[argc] = NULL; /* 注意: 最后一个元素要设置为空指针 */
        }
    }
    return argv;
}
} // namespace utility
