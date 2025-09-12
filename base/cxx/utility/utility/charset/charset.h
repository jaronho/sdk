#pragma once
#include <string>
#include <vector>

namespace utility
{
/**
 * @brief 字符集
 */
class Charset final
{
public:
    enum class Coding
    {
        unknown = -1,
        utf8,
        utf8_bom,
        gbk
    };

public:
    /**
     * @brief 是否为ASCII
     * @param str 字符串
     * @return true-是, false-否
     */
    static bool isAscii(const std::string& str);

    /**
     * @brief 获取当前默认地域编码
     * @return 编码, 例如: Linux下为'zh_CN.UTF-8', Windows下为'Chinese (Simplified)_China.936'
     */
    static std::string getLocale();

    /**
     * @brief 设置当前默认地域编码
     * @return true-成功, false-失败
     */
    static bool setLocale(const std::string& locale);

#ifdef _WIN32
    /**
     * @brief 字符串转为宽字符串
     * @param str 字符串
     * @param codePage 编码, 值: 0.CP_ACP, 1.CP_OEMCP, 65001.CP_UTF8
     * @return 宽字符串
     */
    static std::wstring string2wstring(const std::string& str, size_t codePage = 65001);

    /**
     * @brief 宽字符串转为字符串)
     * @param wstr 宽字符串
     * @param codePage 编码, 值: 0.CP_ACP, 1.CP_OEMCP, 65001.CP_UTF8
     * @return 字符串
     */
    static std::string wstring2string(const std::wstring& wstr, size_t codePage = 65001);
#endif

    /**
     * @brief 获取编码
     * @param str 字符串
     * @param nonAsciiChars 非ASCII字符列表, 列表size表示非ASCII字符总个数, value表示每个字符所占的字节数
     * @return 编码
     */
    static Coding getCoding(const std::string& str, std::vector<unsigned int>* nonAsciiChars = nullptr);

    /**
     * @brief UTF8转Unicode
     * @param str UTF8字符串
     * @return Unicode字符串
     */
    static std::wstring utf8ToUnicode(const std::string& str);

    /**
     * @brief Unicode转UTF8
     * @param wstr Unicode字符串
     * @return UTF8字符串
     */
    static std::string unicodeToUtf8(const std::wstring& wstr);

    /**
     * @brief GBK转Unicode
     * @param str GBK字符串
     * @return Unicode字符串
     */
    static std::wstring gbkToUnicode(const std::string& str);

    /**
     * @brief Unicode转GBK
     * @param wstr Unicode字符串
     * @return GBK字符串
     */
    static std::string unicodeToGbk(const std::wstring& wstr);

    /**
     * @brief GBK转UTF8
     * @param str GBK字符串
     * @return UTF8字符串
     */
    static std::string gbkToUtf8(const std::string& str);

    /**
     * @brief UTF8转GBK
     * @param str UTF8字符串
     * @return GBK字符串
     */
    static std::string utf8ToGbk(const std::string& str);

    /**
     * @brief 把 "\xe6\x96\xb0" 这样的 6-char 序列转成 3-byte UTF-8
     * @param in UTF8编码的汉字被错误地以转义形式显示的字符串(UTF8字节当作Latin-1或ASCII字符串来处理)
     * @return UTF8字符串
     */
    static std::string unescapeToUtf8(const std::string& in);
};
} // namespace utility
