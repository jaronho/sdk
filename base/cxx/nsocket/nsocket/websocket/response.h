#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

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
     * @param resp 响应对象
     * @param secWebSocketKey key值
     * @param data [输出]响应数据
     */
    static void create(Response resp, const std::string& secWebSocketKey, std::vector<unsigned char>& data);

private:
    /**
     * @brief 计算Accept值
     * @param secWebSocketKey key值
     * @return Accept值
     */
    static std::string calcSecWebSocketAccept(const std::string& secWebSocketKey);

public:
    CaseInsensitiveMultimap headers; /* 头部 */
};
using RESPONSE_PTR = std::shared_ptr<Response>;
} // namespace ws
} // namespace nsocket
