#pragma once
#include "json.hpp"

namespace nlohmann
{
namespace impl
{
/**
 * @brief 判断字符串是否为UTF编码
 * @param str 字符串
 * @return true-是, false-否
 */
static bool isUtf8(const std::string& str)
{
    unsigned int byteCount = 0; /* UTF8可用1-6个字节编码, ASCII用1个字节 */
    for (size_t i = 0, len = str.size(); i < len; ++i)
    {
        unsigned char ch = str[i];
        if ('\0' == ch)
        {
            break;
        }
        if (0 == byteCount)
        {
            if (ch >= 0x80) /* 如果不是ASCII码, 应该是多字节符, 计算字节数 */
            {
                if (ch >= 0xFC && ch <= 0xFD)
                {
                    byteCount = 6;
                }
                else if (ch >= 0xF8)
                {
                    byteCount = 5;
                }
                else if (ch >= 0xF0)
                {
                    byteCount = 4;
                }
                else if (ch >= 0xE0)
                {
                    byteCount = 3;
                }
                else if (ch >= 0xC0)
                {
                    byteCount = 2;
                }
                else
                {
                    return false;
                }
                byteCount--;
            }
        }
        else
        {
            if (0x80 != (ch & 0xC0)) /* 多字节符的非首字节, 应为10xxxxxx */
            {
                return false;
            }
            byteCount--; /* 减到为零为止 */
        }
    }
    return (0 == byteCount); /* 非0违反UTF8编码规则 */
}
} // namespace impl

/**
 * @brief 解析字符串
 * @param str json字符串
 * @param j [输出]转换后的json对象
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
static bool parse(const std::string& str, json& j, std::string* errDesc = nullptr)
{
    std::string errorDescribe;
    try
    {
        j = json::parse(str);
        return true;
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    return false;
}

/**
 * @brief 解析字符串
 * @param str json字符串
 * @param errDesc [输出]错误描述(选填)
 * @return json对象
 */
static json parse(const std::string& str, std::string* errDesc = nullptr)
{
    json j;
    parse(str, j, errDesc);
    return j;
}

/**
 * @brief json对象直接转换成对应类型值
 * @tparam ValueType 类型名
 * @param j json对象
 * @param value [输出]类型值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename ValueType>
static bool toValue(const nlohmann::json& j, ValueType& value, std::string* errDesc = nullptr)
{
    std::string errorDescribe;
    try
    {
        if (!j.is_null() || std::is_enum<ValueType>::value)
        {
            j.get_to(value);
            return true;
        }
        errorDescribe = "null";
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    return false;
}

/**
 * @brief json对象直接转换成对应类型值
 * @tparam ValueType 类型名
 * @param j json对象
 * @param errDesc [输出]错误描述(选填)
 * @return 对应类型值
 */
template<typename ValueType>
static ValueType toValue(const nlohmann::json& j, std::string* errDesc = nullptr)
{
    ValueType value;
    toValue(j, value, errDesc);
    return value;
}

/**
 * @brief 获取子项值
 * @tparam ValueType 要获取的类型
 * @param j json对象
 * @param key 子项的key
 * @param value [输出]子项值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename ValueType>
static bool getter(const json& j, const std::string& key, ValueType& value, std::string* errDesc = nullptr)
{
    std::string errorDescribe;
    try
    {
        auto iter = j.find(key);
        if (j.end() != iter)
        {
            if (!iter.value().is_null() || std::is_enum<ValueType>::value)
            {
                iter.value().get_to(value);
                return true;
            }
            errorDescribe = "null";
        }
        else
        {
            errorDescribe = "unfound";
        }
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    return false;
}

/**
 * @brief 获取子项值
 * @tparam ValueType 要获取的类型
 * @param j json对象
 * @param key 子项的key
 * @param errDesc [输出]错误描述(选填)
 * @return 子项值
 */
template<typename ValueType>
static ValueType getter(const json& j, const std::string& key, std::string* errDesc = nullptr)
{
    ValueType value;
    getter(j, key, value, errDesc);
    return value;
}

/**
 * @brief 设置子项值(异常时会自动填充默认值)
 * @tparam ValueType 要设置的类型
 * @param j [输出]json对象
 * @param key 子项的key
 * @param value 子项值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename ValueType>
static bool setter(json& j, const std::string& key, const ValueType& value, std::string* errDesc = nullptr)
{
    std::string errorDescribe;
    try
    {
        if (typeid(std::string) != typeid(value) || impl::isUtf8(*((std::string*)(&value))))
        {
            j[key] = value;
            return true;
        }
        errorDescribe = "invalid UTF-8 value";
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    j[key] = ValueType{};
    return false;
}

/**
 * @brief 追加子项值
 * @tparam ValueType 要追加的类型
 * @param j [输出]json对象
 * @param value 子项值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename ValueType>
static bool append(json& j, const ValueType& value, std::string* errDesc = nullptr)
{
    std::string errorDescribe;
    try
    {
        if (typeid(std::string) != typeid(value) || impl::isUtf8(*((std::string*)(&value))))
        {
            j.emplace_back(value);
            return true;
        }
        errorDescribe = "invalid UTF-8 value";
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    return false;
}

/**
 * @brief 转换为json字符串
 * @param j json对象
 * @param str [输出]json字符串 
 * @param errDesc [输出]错误描述(选填)
 * @param indent 缩进个数(选填), 默认不缩进
 * @param indentChar 缩进字符(选填), 默认1个空格符
 * @param ensureAscii 是否确保ASCII(选填), 默认否
 * @return true-成功, false-失败
 */
static bool dump(const json& j, std::string& str, std::string* errDesc = nullptr, int indent = -1, char indentChar = ' ',
                 bool ensureAscii = false)
{
    std::string errorDescribe;
    try
    {
        str = j.dump(indent, indentChar, ensureAscii, nlohmann::detail::error_handler_t::replace);
        return true;
    }
    catch (const std::exception& e)
    {
        errorDescribe = e.what();
    }
    catch (...)
    {
        errorDescribe = "unknown exception";
    }
    if (errDesc)
    {
        *errDesc = errorDescribe;
    }
    return false;
}

/**
 * @brief 转换为json字符串
 * @param j json对象
 * @param errDesc [输出]错误描述(选填)
 * @param indent 缩进个数(选填), 默认不缩进
 * @param indentChar 缩进字符(选填), 默认1个空格符
 * @param ensureAscii 是否确保ASCII(选填), 默认否
 * @return json字符串
 */
static std::string dump(const json& j, std::string* errDesc = nullptr, int indent = -1, char indentChar = ' ', bool ensureAscii = false)
{
    std::string str;
    dump(j, str, errDesc, indent, indentChar, ensureAscii);
    return str;
}
} // namespace nlohmann
