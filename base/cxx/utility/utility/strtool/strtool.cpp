#include "strtool.h"

#include <algorithm>
#include <string.h>

namespace
{
/**
 * @brief 大写字母表
 */
struct UpperTable
{
    char t[256] = {};
    UpperTable()
    {
        for (int i = 0; i < 256; ++i)
        {
            t[i] = (char)((i >= 'a' && i <= 'z') ? (i - ('a' - 'A')) : i);
        }
    }
};
static const UpperTable s_upper{};

inline char toUpperChar(unsigned char c)
{
    return s_upper.t[c];
}

/**
 * @brief 小写字母表
 */
struct LowerTable
{
    char t[256] = {};
    LowerTable()
    {
        for (int i = 0; i < 256; ++i)
        {
            t[i] = (char)((i >= 'A' && i <= 'Z') ? (i + ('a' - 'A')) : i);
        }
    }
};
static const LowerTable s_lower{};

inline char toLowerChar(unsigned char c)
{
    return s_lower.t[c];
}

inline unsigned char hexVal(unsigned char c)
{
    c = (unsigned char)(toUpperChar(c));
    if (c >= '0' && c <= '9')
    {
        return (unsigned char)(c - '0');
    }
    else if (c >= 'A' && c <= 'F')
    {
        return (unsigned char)(c - 'A' + 10);
    }
    return 0xFF;
}
} // namespace

namespace utility
{
void StrTool::trimLeft(std::string& str, char c)
{
    size_t pos = str.find_first_not_of(c);
    if (std::string::npos == pos)
    {
        str.clear();
    }
    else if (pos > 0)
    {
        str.erase(0, pos);
    }
}

void StrTool::trimRight(std::string& str, char c)
{
    size_t pos = str.find_last_not_of(c);
    if (std::string::npos == pos)
    {
        str.clear();
    }
    else
    {
        str.erase(pos + 1);
    }
}

void StrTool::trimLeftRight(std::string& str, char c)
{
    trimLeft(str, c);
    trimRight(str, c);
}

void StrTool::trimDuplicate(std::string& str, char c)
{
    size_t strCount = str.size();
    if (strCount < 2) /*  0或1个字符无需去重 */
    {
        return;
    }
    size_t w = 0; /* 写指针 */
    bool prevIsC = false; /* 上一个"写入"的字符是否是c */
    for (size_t r = 0; r < strCount; ++r)
    {
        char ch = str[r];
        if (ch == c)
        {
            if (prevIsC) /* 连续重复的 c，跳过 */
            {
                continue;
            }
            prevIsC = true;
        }
        else
        {
            prevIsC = false;
        }
        str[w++] = ch; /* 原地写 */
    }
    str.resize(w); /* 截断尾部多余部分 */
}

std::string StrTool::toUpper(const std::string& str)
{
    std::string result(str);
    for (size_t i = 0, strCount = result.size(); i < strCount; ++i)
    {
        result[i] = toUpperChar((unsigned char)(result[i]));
    }
    return result;
}

void StrTool::toUpperInPlace(std::string& str)
{
    for (size_t i = 0, strCount = str.size(); i < strCount; ++i)
    {
        str[i] = toUpperChar((unsigned char)(str[i]));
    }
}

std::string StrTool::toLower(const std::string& str)
{
    std::string result(str);
    for (size_t i = 0, strCount = result.size(); i < strCount; ++i)
    {
        result[i] = toLowerChar((unsigned char)(result[i]));
    }
    return result;
}

void StrTool::toLowerInPlace(std::string& str)
{
    for (size_t i = 0, strCount = str.size(); i < strCount; ++i)
    {
        str[i] = toLowerChar((unsigned char)(str[i]));
    }
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
    if (str.empty() || sep.empty() || !itemFunc)
    {
        return;
    }
    size_t strCount = str.size();
    size_t sepCount = sep.size();
    size_t i = 0;
    while (i <= strCount)
    {
        size_t pos = str.find(sep, i);
        if (std::string::npos == pos)
        {
            pos = strCount;
        }
        itemFunc(str.substr(i, pos - i));
        i = pos + sepCount;
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
    uint32_t count = 0;
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

bool StrTool::equal(const std::string& str1, const std::string& str2, bool caseSensitive)
{
    if (caseSensitive)
    {
        return (str1 == str2);
    }
    size_t count = str1.size();
    if (str2.size() != count)
    {
        return false;
    }
    for (size_t i = 0; i < count; ++i)
    {
        unsigned char c1 = (unsigned char)(str1[i]);
        unsigned char c2 = (unsigned char)(str2[i]);
        if (c1 != c2 && toLowerChar(c1) != toLowerChar(c2))
        {
            return false;
        }
    }
    return true;
}

size_t StrTool::indexOf(const std::string& str, const std::string& pattern, size_t offset, bool caseSensitive)
{
    if (pattern.empty())
    {
        return std::string::npos;
    }
    size_t strCount = str.size();
    size_t patternCount = pattern.size();
    if (patternCount > strCount)
    {
        return std::string::npos;
    }
    if (std::string::npos == offset)
    {
        offset = 0;
    }
    if (offset > strCount - patternCount) /* 剩余长度不足，不可能匹配 */
    {
        return std::string::npos;
    }
    if (caseSensitive)
    {
        return str.find(pattern, offset);
    }
    size_t last = strCount - patternCount; /* 起点最大可取last(含), 此时恰好匹配到末尾 */
    for (size_t i = offset; i <= last; ++i)
    {
        size_t j = 0;
        for (; j < patternCount; ++j)
        {
            unsigned char a = (unsigned char)(str[i + j]);
            unsigned char b = (unsigned char)(pattern[j]);
            if (a != b && toLowerChar(a) != toLowerChar(b))
            {
                break;
            }
        }
        if (j == patternCount) /* 匹配成功, 返回起点 */
        {
            return i;
        }
    }
    return std::string::npos;
}

bool StrTool::contains(const std::string& str, const std::string& pattern, bool caseSensitive, bool wholeWord)
{
    if (pattern.empty())
    {
        return true;
    }
    size_t strCount = str.size();
    size_t patternCount = pattern.size();
    if (patternCount > strCount)
    {
        return false;
    }
    /* 定位首个匹配(大小写敏感与否统一在此处理) */
    size_t begPos = std::string::npos;
    if (caseSensitive)
    {
        begPos = str.find(pattern);
    }
    else
    {
        size_t last = strCount - patternCount;
        for (size_t i = 0; i <= last; ++i)
        {
            size_t j = 0;
            for (; j < patternCount; ++j)
            {
                unsigned char a = (unsigned char)(str[i + j]);
                unsigned char b = (unsigned char)(pattern[j]);
                if (a != b && toLowerChar(a) != toLowerChar(b))
                {
                    break;
                }
            }
            if (j == patternCount)
            {
                begPos = i;
                break;
            }
        }
    }
    if (std::string::npos == begPos)
    {
        return false;
    }
    if (wholeWord)
    {
        /* 左侧紧邻字符 */
        if (begPos > 0)
        {
            unsigned char pch = (unsigned char)(str[begPos - 1]);
            if (pch < 32 || pch > 126 || (pch >= '0' && pch <= '9') || (pch >= 'A' && pch <= 'Z') || (pch >= 'a' && pch <= 'z'))
            {
                return false;
            }
        }
        /* 右侧紧邻字符 */
        size_t endPos = begPos + patternCount - 1; /* pattern 最后一个字符的下标 */
        if (endPos + 1 < strCount)
        {
            unsigned char nch = (unsigned char)(str[endPos + 1]);
            if (nch < 32 || nch > 126 || (nch >= '0' && nch <= '9') || (nch >= 'A' && nch <= 'Z') || (nch >= 'a' && nch <= 'z'))
            {
                return false;
            }
        }
    }
    return true;
}

bool StrTool::isBeginWith(const std::string& str, const std::string& beg, bool caseSensitive)
{
    if (beg.empty())
    {
        return true;
    }
    size_t strCount = str.size();
    size_t begCount = beg.size();
    if (begCount > strCount)
    {
        return false;
    }
    if (caseSensitive)
    {
        return (0 == str.compare(0, begCount, beg));
    }
    for (size_t i = 0; i < begCount; ++i)
    {
        unsigned char c1 = (unsigned char)(str[i]);
        unsigned char c2 = (unsigned char)(beg[i]);
        if (c1 != c2 && toLowerChar(c1) != toLowerChar(c2))
        {
            return false;
        }
    }
    return true;
}

bool StrTool::isEndWith(const std::string& str, const std::string& end, bool caseSensitive)
{
    if (end.empty())
    {
        return true;
    }
    size_t strCount = str.size();
    size_t endCount = end.size();
    if (endCount > strCount)
    {
        return false;
    }
    if (caseSensitive)
    {
        return (0 == str.compare(strCount - endCount, endCount, end));
    }
    const char* s = str.data() + (strCount - endCount);
    const char* e = end.data();
    for (size_t i = 0; i < endCount; ++i)
    {
        unsigned char c1 = (unsigned char)(s[i]);
        unsigned char c2 = (unsigned char)(e[i]);
        if (c1 != c2 && toLowerChar(c1) != toLowerChar(c2))
        {
            return false;
        }
    }
    return true;
}

size_t StrTool::findCount(const std::string& str, const std::string& pattern, bool caseSensitive, bool wholeWord)
{
    if (pattern.empty())
    {
        return 0;
    }
    size_t strCount = str.size();
    size_t patternCount = pattern.size();
    if (patternCount > strCount)
    {
        return 0;
    }
    /* 步进: wholeWord=true不重叠(跳过整个pattern), =false允许重叠(前进1) */
    size_t step = wholeWord ? patternCount : 1;
    size_t last = strCount - patternCount; /* 匹配起点最大下标 */
    size_t count = 0, i = 0;
    if (caseSensitive)
    {
        while (i <= last)
        {
            size_t bpos = str.find(pattern, i);
            if (bpos == std::string::npos || bpos > last)
            {
                break;
            }
            ++count;
            i = bpos + step;
        }
        return count;
    }
    while (i <= last)
    {
        size_t j = 0;
        for (; j < patternCount; ++j)
        {
            unsigned char a = (unsigned char)(str[i + j]);
            unsigned char b = (unsigned char)(pattern[j]);
            if (a != b && toLowerChar(a) != toLowerChar(b))
            {
                break;
            }
        }
        if (j == patternCount) /* 命中: 按step前进 */
        {
            ++count;
            i += step;
        }
        else /* 未命中: 前进1 */
        {
            ++i;
        }
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
    static const char* digitsLower = "0123456789abcdef";
    static const char* digitsUpper = "0123456789ABCDEF";
    const char* digits = upper ? digitsUpper : digitsLower;
    size_t sepCount = sep.size();
    /* 预估总长度: 每字节2位 + 分隔符 */
    size_t total = (size_t)byteCount * 2;
    if (sepCount > 0 && byteCount > 1)
    {
        total += ((size_t)byteCount - 1) * sepCount;
    }
    hexStr.reserve(total);
    for (size_t i = 0; i < byteCount; ++i)
    {
        if (i > 0 && sepCount > 0)
        {
            hexStr += sep;
        }
        unsigned char b = (unsigned char)(bytes[i]);
        hexStr.push_back(digits[b >> 4]); /* 高 4 位 */
        hexStr.push_back(digits[b & 0x0F]); /* 低 4 位 */
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
        size_t strCount = hexStr.size();
        if (0 != strCount % 2) /* 必须偶数长度 */
        {
            return bytes;
        }
        bytes.resize(strCount / 2);
        for (size_t i = 0; i < strCount; i += 2)
        {
            unsigned char h = hexVal((unsigned char)(hexStr[i]));
            unsigned char l = hexVal((unsigned char)(hexStr[i + 1]));
            if (0xFF == h || 0xFF == l) /* 保持原行为: 返回已解析部分 */
            {
                return bytes;
            }
            bytes[i / 2] = (char)((h << 4) | l);
        }
    }
    else /* 有分隔符 */
    {
        size_t strCount = hexStr.size();
        size_t sepCount = sep.size();
        bytes.reserve(strCount / (sepCount + 1) + 1);
        size_t i = 0;
        while (i < strCount)
        {
            size_t pos = hexStr.find(sep, i);
            if (std::string::npos == pos)
            {
                pos = strCount;
            }
            size_t len = pos - i;
            const char* seg = hexStr.data() + i;
            /* 处理 0x / 0X 前缀 */
            size_t start = 0;
            if ((3 == len || 4 == len) && '0' == seg[0] && ('x' == seg[1] || 'X' == seg[1]))
            {
                start = 2;
                len -= 2;
            }
            if (len < 1 || len > 2) /* 非法, 保持原行为 */
            {
                return bytes;
            }
            unsigned char h, l;
            if (2 == len)
            {
                h = hexVal((unsigned char)(seg[start]));
                l = hexVal((unsigned char)(seg[start + 1]));
            }
            else
            {
                h = 0;
                l = hexVal((unsigned char)(seg[start]));
            }
            if (0xFF == h || 0xFF == l)
            {
                return bytes;
            }
            bytes.push_back((char)((h << 4) | l));
            i = pos + sepCount;
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
    size_t count = length - str.size();
    std::string result;
    result.reserve(length);
    if (leftFlag)
    {
        result.append(count, c);
        result.append(str);
    }
    else
    {
        result.append(str);
        result.append(count, c);
    }
    return result;
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
