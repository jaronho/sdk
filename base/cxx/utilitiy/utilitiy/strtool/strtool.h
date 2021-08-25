#pragma once
#include <string>
#include <vector>

namespace utilitiy
{
class StrTool final
{
public:
    /**
     * @brief 移除左边字符
     * @param str [输入/输出]字符串
     * @param c 要移除的字符
     */
    static void trimLeft(std::string& str, char c);

    /**
     * @brief 移除右边字符
     * @param str [输入/输出]字符串
     * @param c 要移除的字符
     */
    static void trimRight(std::string& str, char c);

    /**
     * @brief 移除左右两边空格
     * @param str [输入/输出]字符串
     */
    static void trimLeftRightSpace(std::string& str);

    /**
     * @brief 转为小写
     * @param str 字符串
     * @param 小写字符串
     */
    static std::string toLower(std::string str);

    /**
     * @brief 转为大写
     * @param str 字符串
     * @return 大写字符串
     */
    static std::string toUpper(std::string str);

    /**
     * @brief 内容替换
     * @param str 字符串
     * @param rep 被替换的内容
     * @param dest 替换后的内容
     * @return 替换后的字符串
     */
    static std::string replace(std::string str, const std::string& rep, const std::string& dest);

    /**
     * @brief 分割
     * @param str 字符串
     * @param pattern 分割的符号
     * @return 分割后的子字符串列表
     */
    static std::vector<std::string> split(const std::string& str, const std::string& pattern);

    /**
     * @brief 组合
     * @param strList 字符串列表
     * @param pattern 组合的符号(选填), 默认为空
     * @return 组合后的字符串
     */
    static std::string join(const std::vector<std::string>& strList, const std::string& pattern = "");

    /**
     * @brief 是否以指定字符串开头
     * @param str 字符串
     * @param beg 开头字符串
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @return true-是, false-否
     */
    static bool isBeginWith(std::string str, std::string beg, bool caseSensitive = true);

    /**
     * @brief 是否以指定字符串结尾
     * @param str 字符串
     * @param end 结尾字符串
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @return true-是, false-否
     */
    static bool isEndWith(std::string str, std::string end, bool caseSensitive = true);

    /**
     * @brief 转为16进制字符串
     * @param bytes 字节流
     * @param byteCount 字节数
     * @param upper 是否转为大写(选填), 默认为大写
     * @param sep 每个16进制间的分隔符(选填)
     * @return 16进制字符串
     */
    static std::string toHex(const char* bytes, unsigned int byteCount, bool upper = true, const std::string& sep = "");

    /**
     * @brief 16进制字符串转为字节流
     * @param hexStr 16进制字符串
     * @param sep 每个16进制间的分隔符(选填)
     * @return 字节流
     */
    static std::vector<char> fromHex(const std::string& hexStr, const std::string& sep = "");

    /**
     * @brief 是否为ASCII字符串
     * @param str 字符串
     * @return true-是, false-否
     */
    static bool isAscii(const std::string& str);
};
} // namespace utilitiy
