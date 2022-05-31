#pragma once
#include <string>

namespace utility
{
class Digit final
{
public:
    /**
     * @brief 检查是否为二进制字符串
     * @param binStr 二进制字符串, 例如: "1010"
     * @return true-是, false-否
     */
    static bool isBin(const std::string& binStr);

    /**
     * @brief 检查是否为十六进制字符串
     * @param binStr 十六进制字符串, 例如: "FF88FF", "0xFF88FF"
     * @return true-是, false-否
     */
    static bool isHex(const std::string& hexStr);

    /**
     * @brief 十进制数字转二进制字符串
     * @param dec 十进制数字
     * @param bit 二进制字符串的位数(选填), 不足的则补0
     * @return 二进制字符串
     */
    static std::string dec2bin(unsigned int dec, unsigned int bit = 0);

    /**
     * @brief 二进制字符串转十进制数字
     * @param binStr 二进制字符串
     * @return 十进制数字
     */
    static unsigned int bin2dec(const std::string& binStr);

    /**
     * @brief 十进制数字转十六进制字符串
     * @param dec 十进制数字
     * @param bit 十六进制字符串的位数(选填), 不足的则补0
     * @param prefix 是否加上前缀"0x"(选填), 默认无
     * @param isUpper 是否输出大写(选填), 默认大写
     * @return 十六进制字符串
     */
    static std::string dec2hex(unsigned int dec, unsigned int bit = 0, bool prefix = false, bool isUpper = true);

    /**
     * @brief 十六进制字符串转十进制数字
     * @param hexStr 十六进制字符串
     * @return 十进制数字
     */
    static unsigned int hex2dec(const std::string& hexStr);
};
} // namespace utility
