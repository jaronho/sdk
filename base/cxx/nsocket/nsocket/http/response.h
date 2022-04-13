#pragma once
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
     * @param resp 响应对象
     * @param data [输出]响应数据
     */
    static void pack(Response resp, std::vector<unsigned char>& data);

public:
    std::string version; /* 版本 */
    StatusCode statusCode = StatusCode::success_ok; /* 状态码 */
    CaseInsensitiveMultimap headers; /* 头部 */
    std::vector<unsigned char> body; /* 包体, 当有业务数据返回时需要填充该字段 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace http
} // namespace nsocket
