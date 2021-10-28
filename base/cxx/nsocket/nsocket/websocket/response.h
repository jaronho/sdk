#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "../http_status_code.h"
#include "../multimap.hpp"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket响应
 */
class Response
{
public:
    /**
     * @brief 创建响应数据
     * @param data [输出]响应数据
     * @param secWebSocketKey key值
     */
    void create(std::vector<unsigned char>& data, const std::string& secWebSocketKey);

private:
    /**
     * @brief 计算Accept值
     * @param secWebSocketKey key值
     * @return Accept值
     */
    std::string calcSecWebSocketAccept(const std::string& secWebSocketKey);

public:
    std::string version = "HTTP/1.1"; /* 版本 */
    HttpStatusCode statusCode = HttpStatusCode::information_switching_protocols; /* 状态码 */
    CaseInsensitiveMultimap headers; /* 头部 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace ws
} // namespace nsocket
