#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../multimap.hpp"
#include "status_code.h"

namespace nsocket
{
namespace http
{
/**
 * @brief HTTP响应
 */
class Response
{
public:
    /**
     * @brief 打包响应数据(把对象转为字节流)
     * @return 响应数据
     */
    std::vector<unsigned char> pack();

public:
    std::string version; /* 版本 */
    StatusCode statusCode = StatusCode::success_ok; /* 状态码 */
    CaseInsensitiveMultimap headers; /* 头部 */
    std::vector<unsigned char> body; /* 包体, 当有业务数据返回时需要填充该字段 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;

/**
 * @brief 获取文件对应的MIME类型
 * @param fileName 文件名
 * @return MIME类型
 */
std::string getFileMimeType(const std::string& fileName);

/**
 * @brief 创建200响应 
 */
RESPONSE_PTR makeResponse200();

/**
 * @brief 创建404响应
 */
RESPONSE_PTR makeResponse404();

/**
 * @brief 创建405响应
 */
RESPONSE_PTR makeResponse405();
} // namespace http
} // namespace nsocket
