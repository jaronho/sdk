#pragma once

#include "curl_object.h"
#include "curl_request.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace curlex
{
/**
 * @brief 函数集
 */
struct FuncSet
{
    /**
     * @brief 是否停止函数
     * @return true-停止, false-继续
     */
    std::function<bool()> isStopFunc = nullptr;

    /**
     * @brief 发送进度函数
     * @param now 当前进度(字节)
     * @param total 总进度(字节)
     * @param speed 速度(字节/秒)
     */
    std::function<void(int64_t now, int64_t total, double speed)> sendProgressFunc = nullptr;

    /**
     * @brief 接收进度函数
     * @param now 当前进度(字节)
     * @param total 总进度(字节)
     * @param speed 速度(字节/秒)
     */
    std::function<void(int64_t now, int64_t total, double speed)> recvProgressFunc = nullptr;

    CurlDebugFunc debugFunc = nullptr; /* 调试函数 */
};

/**
 * @brief 响应数据
 */
struct Response
{
    int curlCode = -1; /* curl码 */
    std::string errorDesc; /* 错误描述 */
    int httpCode = -1; /* http码 */
    std::map<std::string, std::string> headers; /* 响应头 */
    std::string body; /* 响应体 */
};

/**
 * @brief DELETE请求
 * @param req 请求参数
 * @param funcSet 函数集
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlDelete(const RequestPtr& req, const FuncSet& funcSet, Response& resp);

/**
 * @brief GET请求
 * @param req 请求参数
 * @param funcSet 函数集
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlGet(const RequestPtr& req, const FuncSet& funcSet, Response& resp);

/**
 * @brief PUT请求
 * @param req 请求参数
 * @param funcSet 函数集
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlPut(const RequestPtr& req, const FuncSet& funcSet, Response& resp);

/**
 * @brief POST请求
 * @param req 请求参数
 * @param funcSet 函数集
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlPost(const RequestPtr& req, const FuncSet& funcSet, Response& resp);

/**
 * @brief 下载文件
 * @param req 请求参数
 * @param filename 要保存的本地文件名
 * @param recover 是否强制覆盖(true-强制覆盖,false-若本地文件已存在会进行断点续传)
 * @param funcSet 函数集
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlDownload(const RequestPtr& req, const std::string& filename, bool recover, const FuncSet& funcSet, Response& resp);
} // namespace curlex
