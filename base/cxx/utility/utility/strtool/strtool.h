#pragma once
#include <string>
#include <vector>

namespace utility
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
     * @brief 移除左右两边字符
     * @param str [输入/输出]字符串
     * @param c 要移除的字符(选填), 默认空格
     */
    static void trimLeftRight(std::string& str, char c = ' ');

    /**
     * @brief 移除连续重复字符, 例如: aaabbccddaaabcd 对 a 进行去重得到 abbccddabcd
     * @param str [输入/输出]字符串
     * @param c 要移除的字符
     */
    static void trimDuplicate(std::string& str, char c);

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
     * @param sep 分割的符号
     * @return 分割后的子字符串列表
     */
    static std::vector<std::string> split(const std::string& str, const std::string& sep);

    /**
     * @brief 分割
     * @param str 字符串
     * @param sepNum 分割的数量
     * @return 分割后的子字符串列表
     */
    static std::vector<std::string> split(const std::string& str, int sepNum);

    /**
     * @breif 截取字符串
     * @param str 原始字符串
     * @param sep 分割符
     * @param maxCount 允许的最多个数
     * @param output [输出]分割后的字符串列表
     * @return 最终分割个数
     */
    static uint32_t split(char* str, const char* sep, uint32_t maxCount, char* output[]);

    /**
     * @brief 组合
     * @param strList 字符串列表
     * @param sep 组合的符号(选填), 默认为空
     * @param count 指定要组合字符串列表中的元素个数(选填), 默认为0表示组合所有元素
     * @return 组合后的字符串
     */
    static std::string join(const std::vector<std::string>& strList, const std::string& sep = "", size_t count = 0);

    /**
     * @brief 判断两个字符串是否相等
     * @param str1 字符串1
     * @param str2 字符串2
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @return true-相等, false-不相等
     */
    static bool equal(std::string str1, std::string str2, bool caseSensitive = true);

    /**
     * @brief 获取指定字符串的位置
     * @param str 字符串
     * @param pattern 字符串
     * @param offset 开始查找位置(选填), 默认从0开始
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @return 位置
     */
    static size_t indexOf(std::string str, std::string pattern, size_t offset = 0, bool caseSensitive = true);

    /**
     * @brief 是否包含指定字符串
     * @param str 字符串
     * @param pattern 字符串
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @param wholeWord 是否全词匹配(选填), true-是(例如: "aaa"中无法匹配"aa"), false-否(例如: "aaa"中可以匹配"aa")
     * @return true-是, false-否
     */
    static bool contains(std::string str, std::string pattern, bool caseSensitive = true, bool wholeWord = false);

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
     * @brief 查找指定字符串个数
     * @param str 字符串
     * @param pattern 字符串
     * @param caseSensitive 是否区分大小写(选填), true-区分大小写, false-不区分
     * @param wholeWord 是否全词匹配(选填), true-是(例如: "aaa"中可找到1个"aa"), false-否(例如: "aaa"中可找到2个"aa")
     * @return 找到的个数
     */
    static size_t findCount(std::string str, std::string pattern, bool caseSensitive = true, bool wholeWord = false);

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
     * @brief 字符串补位
     * @param str 原始字符串
     * @param c 补位字符, 例如: '0'
     * @param length 补位后总长度
     * @param leftFlag true-补位字符串左边, false-补位字符串右边
     * @param 补位后的字符串
     */
    static std::string fillPlace(const std::string& str, char c, size_t length, bool leftFlag = true);

    /**
     * @brief 把字符串数组转换为参数列表
     * @param vec 字符串数组
     * @param argc [输出]参数个数
     * @return 参数列表(需要外部调用free释放内存)
     */
    static char** convertToArgv(const std::vector<std::string>& vec, int& argc);
};
} // namespace utility
