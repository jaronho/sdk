#pragma once
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace nsocket
{
namespace http
{
/**
 * @brief 比较字符串是否相等(忽略大小写)
 * @param str1 字符串1
 * @param str2 字符串2
 * @return true-相等, false-不相等
 */
inline bool case_insensitive_equal(const std::string& str1, const std::string& str2) noexcept
{
    return str1.size() == str2.size()
           && std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b) { return tolower(a) == tolower(b); });
}

/**
 * @brief 字符串比较类(忽略大小写)
 */
class CaseInsensitiveEqual
{
public:
    bool operator()(const std::string& str1, const std::string& str2) const noexcept
    {
        return case_insensitive_equal(str1, str2);
    }
};

/**
 * @brief 哈希值计算类(忽略大小写)
 *        参考: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x/2595226#2595226
 */
class CaseInsensitiveHash
{
public:
    size_t operator()(const std::string& str) const noexcept
    {
        size_t h = 0;
        std::hash<int> hash;
        for (const auto& ch : str)
        {
            h ^= hash(tolower(ch)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

/**
 * @brief 无序关联容器(忽略大小写), 参数: 1-键的类型, 2-值的类型, 3-底层存储键值对时采用的哈希函数, 4-判断各个键值对的键相等的规则
 */
using CaseInsensitiveMultimap = std::unordered_multimap<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual>;
} // namespace http
} // namespace nsocket
