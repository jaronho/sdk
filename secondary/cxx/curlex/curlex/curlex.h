#pragma once

#include "curl_request.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace curlex
{
/**
 * @brief 是否停止函数
 * @return true-停止, false-继续
 */
using IsStopFunc = std::function<bool()>;

/**
 * @brief 进度函数
 * @param now 当前进度(字节)
 * @param total 总进度(字节)
 * @param speed 速度(字节/秒)
 */
using ProgressFunc = std::function<void(int64_t now, int64_t total, double speed)>;

/**
 * @brief 基本响应数据
 */
struct Response
{
    int curlCode = -1; /* curl码 */
    std::string errorDesc; /* 错误描述 */
    int httpCode = -1; /* http码 */
    std::map<std::string, std::string> headers; /* 响应头 */
};

/**
 * @brief 简单响应数据
 */
struct SimpleResponse : public Response
{
    std::string body; /* 响应体 */
};

/**
 * @brief GET请求
 * @param req 请求参数
 * @param isStopFunc 是否停止函数
 * @param recvProgressFunc 接收进度函数
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlGet(const RequestPtr& req, const IsStopFunc& isStopFunc, const ProgressFunc& recvProgressFunc, SimpleResponse& resp);

/**
 * @brief POST请求
 * @param req 请求参数
 * @param isStopFunc 是否停止函数
 * @param sendProgressFunc 发送进度函数
 * @param recvProgressFunc 接收进度函数
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlPost(const RequestPtr& req, const IsStopFunc& isStopFunc, const ProgressFunc& sendProgressFunc,
              const ProgressFunc& recvProgressFunc, SimpleResponse& resp);

/**
 * @brief PUT请求
 * @param req 请求参数
 * @param isStopFunc 是否停止函数
 * @param sendProgressFunc 发送进度函数
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlPut(const RequestPtr& req, const IsStopFunc& isStopFunc, const ProgressFunc& sendProgressFunc, SimpleResponse& resp);

/**
 * @brief DELETE请求
 * @param req 请求参数
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlDelete(const RequestPtr& req, SimpleResponse& resp);

/**
 * @brief 下载文件
 * @param req 请求参数
 * @param filename 要保存的本地文件名
 * @param recover 是否强制覆盖(true-强制覆盖,false-若本地文件已存在会进行断点续传)
 * @param isStopFunc 是否停止函数
 * @param downloadProgressFunc 下载进度函数
 * @param resp 响应数据
 * @return true-成功, false-失败
 */
bool curlDownload(const RequestPtr& req, const std::string& filename, bool recover, const IsStopFunc& isStopFunc,
                  const ProgressFunc& downloadProgressFunc, Response& resp);
} // namespace curlex
