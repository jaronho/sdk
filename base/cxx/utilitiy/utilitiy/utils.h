#pragma once
#include <string>
#include <vector>

namespace utilitiy
{
class Utils final
{
public:
    /**
     * @brief 字符串内容替换
     * @param str 原始字符串
     * @param rep 被替换的内容
     * @param dest 替换后的内容
     * @return 替换后的字符串
     */
    static std::string replaceString(std::string str, const std::string& rep, const std::string& dest);

    /**
     * @brief 字符串分割
     * @param str 原始字符串
     * @param pattern 分割的内容
     * @return 分割后的子字符串列表
     */
    static std::vector<std::string> splitString(std::string str, const std::string& pattern);
};
} // namespace utilitiy
