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

std::string StrTool::replace(std::string str, const std::string& rep, const std::function<std::string(size_t index)>& destFunc)
{
    if (str.empty() || rep.empty() || !destFunc)
    {
        return str;
    }
    size_t index = 0;
    std::string::size_type pos = 0;
    while (std::string::npos != (pos = str.find(rep, pos)))
    {
        const auto& dest = destFunc(index++);
        str.replace(pos, rep.size(), dest);
        pos += dest.size();
    }
    return str;
}

std::string StrTool::replace(std::string str, const std::string& rep, const std::string& dest)
{
    return replace(str, rep, [dest](size_t index) { return dest; });
}

void StrTool::split(const std::string& str, const std::string& sep, const std::function<void(const std::string& item)>& itemFunc)
{
    std::vector<std::string> strList;
    if (str.empty() || sep.empty() || !itemFunc)
    {
        return;
    }
    for (size_t i = 0; i <= str.size();)
    {
        auto pos = str.find(sep, i);
        if (std::string::npos == pos)
        {
            pos = str.size();
        }
        itemFunc(str.substr(i, pos - i));
        i = pos + sep.size();
    }
}

std::vector<std::string> StrTool::split(const std::string& str, const std::string& sep)
{
    std::vector<std::string> strList;
    split(str, sep, [&strList](const std::string& item) { strList.emplace_back(item); });
    return strList;
}

void StrTool::split(const std::string& str, int sepNum, const std::function<void(const std::string& item)>& itemFunc)
{
    if (str.empty() || !itemFunc)
    {
        return;
    }
    if (sepNum > 0)
    {
        std::string sepStr;
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (sepStr.size() == sepNum)
            {
                itemFunc(sepStr);
                sepStr.clear();
            }
            sepStr.push_back(str[i]);
        }
        if (!sepStr.empty())
        {
            itemFunc(sepStr);
        }
    }
    else
    {
        itemFunc(str);
    }
}

std::vector<std::string> StrTool::split(const std::string& str, int sepNum)
{
    std::vector<std::string> strList;
    split(str, sepNum, [&strList](const std::string& item) { strList.emplace_back(item); });
    return strList;
}

uint32_t StrTool::split(char* str, const char* sep, uint32_t maxCount, char* output[])
{
    char* block;
    uint8_t count = 0;
    if (str && maxCount > 0 && output)
    {
        if (sep)
        {
            block = strtok(str, sep);
            while (NULL != block && count < maxCount)
            {
                output[count] = block;
                ++count;
                block = strtok(NULL, sep);
            }
        }
        else
        {
            output[count] = str;
            ++count;
        }
    }
    return count;
}

std::string StrTool::join(const std::vector<std::string>& strList, const std::string& sep, size_t count)
{
    return join<std::string>(
        strList, [](const std::string& item) { return item; }, sep, count);
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

bool StrTool::contains(std::string str, std::string pattern, bool caseSensitive, bool wholeWord)
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
    auto bpos = str.find(pattern);
    if (std::string::npos == bpos)
    {
        return false;
    }
    if (wholeWord)
    {
        /* 判断前面字符是否非打印字符, 或者数字/字母 */
        if (bpos > 0)
        {
            auto pch = str[bpos - 1];
            if (pch < 32 || pch > 126 || (pch >= '0' && pch <= '9') || (pch >= 'A' && pch <= 'Z') || (pch >= 'a' && pch <= 'z'))
            {
                return false;
            }
        }
        /* 判断后面字符是否非打印字符, 或者数字/字母 */
        auto epos = bpos + pattern.size() - 1;
        if (epos < str.size() - 1)
        {
            auto nch = str[epos + 1];
            if (nch < 32 || nch > 126 || (nch >= '0' && nch <= '9') || (nch >= 'A' && nch <= 'Z') || (nch >= 'a' && nch <= 'z'))
            {
                return false;
            }
        }
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

size_t StrTool::findCount(std::string str, std::string pattern, bool caseSensitive, bool wholeWord)
{
    if (!caseSensitive)
    {
        std::transform(str.begin(), str.end(), str.begin(), tolower);
        std::transform(pattern.begin(), pattern.end(), pattern.begin(), tolower);
    }
    size_t count = 0;
    for (size_t i = 0; std::string::npos != (i = str.find(pattern, i));)
    {
        i += (wholeWord ? pattern.size() : 1);
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
