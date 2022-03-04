#pragma once
#include "json.hpp"

namespace nlohmann
{
/**
 * @brief 字符串转为json对象
 * @param str json字符串
 * @param j [输出]转换后的json对象
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
static bool stringToObject(const std::string& str, json& j, std::string* errDesc = nullptr)
{
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        j = json::parse(str);
        return true;
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
    }
    return false;
}

/**
 * @brief 字符串转为json对象
 * @param str json字符串
 * @param errDesc [输出]错误描述(选填)
 * @return json对象
 */
static json stringToObject(const std::string& str, std::string* errDesc = nullptr)
{
    json j;
    stringToObject(str, j, errDesc);
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
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        if (!j.is_null() || std::is_enum<ValueType>::value)
        {
            j.get_to(value);
            return true;
        }
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
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
 * @brief 获取子对象
 * @param j json对象
 * @param key 子项的key
 * @param object [输出]子对象
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
static bool getter(const json& j, const std::string& key, json& object, std::string* errDesc = nullptr)
{
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        auto iter = j.find(key);
        if (j.end() != iter)
        {
            object = iter.value();
            return true;
        }
        else if (errDesc)
        {
            *errDesc = "unfound";
        }
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
    }
    return false;
}

/**
 * @brief 获取子对象
 * @param j json对象
 * @param key 子项的key
 * @param errDesc [输出]错误描述(选填)
 * @return 子对象
 */
static json getter(const json& j, const std::string& key, std::string* errDesc = nullptr)
{
    json object;
    getter(j, key, object, errDesc);
    return object;
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
    if (errDesc)
    {
        (*errDesc).clear();
    }
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
        }
        else if (errDesc)
        {
            *errDesc = "unfound";
        }
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
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
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        j[key] = value;
        return true;
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
    }
    ValueType defValue;
    j[key] = defValue;
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
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        j.emplace_back(value);
        return true;
    }
    catch (const std::exception& e)
    {
        if (errDesc)
        {
            *errDesc = e.what();
        }
    }
    catch (...)
    {
        if (errDesc)
        {
            *errDesc = "unknown exception";
        }
    }
    return false;
}
} // namespace nlohmann
