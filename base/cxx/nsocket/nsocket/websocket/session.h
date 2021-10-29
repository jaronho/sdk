#pragma once
#include <memory>
#include <string>

#include "../tcp/tcp_server.h"
#include "frame.h"
#include "request.h"

namespace nsocket
{
namespace ws
{
/**
 * @brief WebSocket会话
 */
class Session
{
public:
    /**
     * @brief 发送文本
     */
    void sendText(const std::string& text, bool isFin = true);

    /**
     * @brief 发送二进制
     */
    void sendBytes(const std::vector<unsigned char>& bytes, bool isFin = true);

    /**
     * @brief 发送关闭命令(只有1帧)
     */
    void sendClose(const CloseCode& code = CloseCode::close_normal);

    /**
     * @brief 发送ping命令(只有1帧)
     */
    void sendPing();

    /**
     * @brief 发送pong命令(只有1帧)
     */
    void sendPong();

public:
    std::weak_ptr<TcpConnection> wpConn; /* TCP连接 */
    std::shared_ptr<Request> req; /* 请求 */
    std::shared_ptr<Frame> frame; /* 数据帧 */
};
using SESSION_PTR = std::shared_ptr<Session>;
} // namespace ws
} // namespace nsocket
