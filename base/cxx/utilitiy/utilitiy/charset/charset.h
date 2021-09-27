#pragma once
#include <string>

namespace utilitiy
{
class Charset final
{
public:
    enum class Coding
    {
        GBK,
        UTF8,
        UNKOWN
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

    /**
     * @brief 获取编码
     * @param str 字符串
     * @return 编码
     */
    static Coding getCoding(const std::string& str);

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
};
} // namespace utilitiy
