#include "util.h"

#include <string.h>

namespace utility
{
std::string Util::urlEncode(const std::string& srcUrl)
{
    static const char* HEX_CHARS = "0123456789ABCDEF";
    std::string dstUrl;
    dstUrl.reserve(srcUrl.size()); /* 设置最小长度 */
    for (const auto& ch : srcUrl)
    {
        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("-_.~!*'();:@&=+$,/?#[]", ch))
        {
            dstUrl += ch;
        }
        else
        {
            dstUrl += "%";
            dstUrl.push_back(HEX_CHARS[(unsigned char)ch >> 4]);
            dstUrl.push_back(HEX_CHARS[(unsigned char)ch & 15]);
        }
    }
    return dstUrl;
}

std::string Util::urlDecode(const std::string& srcUrl)
{
    std::string dstUrl;
    dstUrl.reserve(srcUrl.size() / 3 + (srcUrl.size() % 3)); /* 设置最小长度 */
    for (size_t i = 0; i < srcUrl.size(); ++i)
    {
        const auto& ch = srcUrl[i];
        if ('%' == ch && i + 2 < srcUrl.size())
        {
            auto hex = srcUrl.substr(i + 1, 2);
            auto decodedCh = (char)std::strtol(hex.c_str(), nullptr, 16);
            dstUrl += decodedCh;
            i += 2;
        }
        else if ('+' == ch)
        {
            dstUrl += ' ';
        }
        else
        {
            dstUrl += ch;
        }
    }
    return dstUrl;
}
} // namespace utility
