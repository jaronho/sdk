#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

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
     * @brief 创建响应数据
     * @param resp 响应对象
     * @param data [输出]响应数据
     */
    static void create(Response resp, std::vector<unsigned char>& data);

public:
    std::string version; /* 版本 */
    StatusCode statusCode = StatusCode::success_ok; /* 状态码 */
    CaseInsensitiveMultimap headers; /* 头部 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace http
} // namespace nsocket
