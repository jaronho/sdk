#pragma once
#include <array>
#include <stdint.h>
#include <string>

namespace algorithm
{
/**
 * @brief UUID生成器
 */
class UUID
{
public:
    /**
     * @brief 生成UUIDv7(二进制格式, 16字节)
     * @return 16字节数组, 可直接存入数据库BINARY(16)
     */
    static std::array<uint8_t, 16> generateUUIDv7();

    /**
     * @brief 生成UUIDv7字符串(36字符, 标准格式)
     * @return 36字符标准UUID字符串, 例如: 018e1234-5678-7abc-8def-0123456789ab
     */
    static std::string generateUUIDv7String();

    /**
     * @brief 快速将16字节UUID写入字符缓冲区
     * @param uuidBytes 16字节数组
     * @param buffer [输出]缓冲区(至少37字节: 36字符 + '\0'), 36字符标准UUID字符串(8-4-4-4-12格式), 例如: 018e1234-5678-7abc-8def-0123456789ab
     */
    static void uuidToString(const std::array<uint8_t, 16>& uuidBytes, char buffer[37]);

    /**
     * @brief 将16字节UUID二进制转换为字符串
     * @param uuidBytes 16字节数组
     * @return 36字符标准UUID字符串, 例如: 018e1234-5678-7abc-8def-0123456789ab
     */
    static std::string uuidToString(const std::array<uint8_t, 16>& uuidBytes);
};
} // namespace algorithm
