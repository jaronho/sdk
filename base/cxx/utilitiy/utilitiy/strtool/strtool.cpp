#include "strtool.h"

#include <algorithm>
#include <string.h>

namespace utilitiy
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

void StrTool::trimLeftRightSpace(std::string& str)
{
    trimLeft(str, ' ');
    trimRight(str, ' ');
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

std::vector<std::string> StrTool::split(const std::string& str, const std::string& pattern)
{
    std::vector<std::string> result;
    if (str.empty() || pattern.empty())
    {
        return result;
    }
    std::string::size_type pos;
    for (size_t i = 0; i < str.size(); ++i)
    {
        pos = str.find(pattern, i);
        if (std::string::npos == pos)
        {
            pos = str.size();
        }
        result.emplace_back(str.substr(i, pos - i));
        i = pos + pattern.size() - 1;
    }
    return result;
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
        sprintf_s(tmp, upper ? "%02X" : "%02x", (unsigned char)bytes[i]);
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

char* StrTool::fromHex(const std::string& hexStr, unsigned int& byteCount, const std::string& sep)
{
    byteCount = 0;
    char* bytes = NULL;
    if (hexStr.empty())
    {
        return bytes;
    }
    if (sep.empty()) /* 无分隔符 */
    {
        if (0 == hexStr.size() % 2) /* 必须保证16进制字符串长度为偶数 */
        {
            byteCount = hexStr.size() / 2;
            bytes = (char*)malloc(sizeof(char) * byteCount);
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
        std::vector<char> byteVec;
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
            byteVec.emplace_back((highByte << 4) | lowByte);
            i = pos + sep.size() - 1;
        }
        byteCount = byteVec.size();
        bytes = (char*)malloc(sizeof(char) * byteCount);
        std::copy(byteVec.begin(), byteVec.end(), bytes);
    }
    return bytes;
}
} // namespace utilitiy
