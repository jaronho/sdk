#pragma once

#include <memory>

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
static bool stringToJson(const std::string& str, json& j, std::string* errDesc = nullptr)
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
static json stringToJson(const std::string& str, std::string* errDesc = nullptr)
{
    json j;
    stringToJson(str, j, errDesc);
    return j;
}

/**
 * @brief json对象直接转换成对应类型值
 * @tparam DataType 类型名
 * @param j json对象
 * @param value [输出]类型值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename DataType>
static bool jsonToValue(const nlohmann::json& j, DataType& value, std::string* errDesc = nullptr)
{
    if (errDesc)
    {
        (*errDesc).clear();
    }
    try
    {
        if (!j.is_null() || std::is_enum<DataType>::value)
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
 * @tparam DataType 类型名
 * @param j json对象
 * @param errDesc [输出]错误描述(选填)
 * @return 对应类型值
 */
template<typename DataType>
static DataType jsonToValue(const nlohmann::json& j, std::string* errDesc = nullptr)
{
    DataType value;
    jsonToValue(j, value, errDesc);
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
static bool getJsonObject(const json& j, const std::string& key, json& object, std::string* errDesc = nullptr)
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
static json getJsonObject(const json& j, const std::string& key, std::string* errDesc = nullptr)
{
    json object;
    getJsonObject(j, key, object, errDesc);
    return object;
}

/**
 * @brief 获取子项值
 * @tparam DataType 要获取的类型
 * @param j json对象
 * @param key 子项的key
 * @param value [输出]子项值
 * @param errDesc [输出]错误描述(选填)
 * @return true-成功, false-失败
 */
template<typename DataType>
static bool getJsonValue(const json& j, const std::string& key, DataType& value, std::string* errDesc = nullptr)
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
            if (!iter.value().is_null() || std::is_enum<DataType>::value)
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
 * @tparam DataType 要获取的类型
 * @param j json对象
 * @param key 子项的key
 * @param errDesc [输出]错误描述(选填)
 * @return 子项值
 */
template<typename DataType>
static bool getJsonValue(const json& j, const std::string& key, std::string* errDesc = nullptr)
{
    DataType value;
    getJsonValue(j, key, value, errDesc);
    return value;
}
} // namespace nlohmann
