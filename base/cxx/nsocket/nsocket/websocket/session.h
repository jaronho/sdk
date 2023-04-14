#pragma once
#include <memory>
#include <string>

#include "../tcp/tcp_connection.h"
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
    friend class Server;

public:
    /**
     * @brief 获取会话ID
     */
    uint64_t getId() const;

    /**
     * @brief 获取客户端主机
     */
    std::string getClientHost() const;

    /**
     * @brief 获取客户端端口
     */
    int getClientPort() const;

    /**
     * @brief 获取URI
     */
    std::string getUri() const;

    /**
     * @brief 发送文本
     */
    void sendText(const std::string& text, bool isFin = true);

    /**
     * @brief 发送二进制
     */
    void sendBytes(const std::vector<unsigned char>& bytes, bool isFin = true);

    /**
     * @brief 发送ping命令
     */
    void sendPing();

    /**
     * @brief 发送pong命令
     */
    void sendPong();

    /**
     * @brief 发送关闭命令
     */
    void sendClose(const CloseCode& code = CloseCode::close_normal);

    /**
     * @brief 当前消息是否为文本
     */
    bool isMsgText() const;

protected:
    size_t m_defaultBufferSize = 0; /* 默认缓冲区大小 */
    std::weak_ptr<TcpConnection> m_wpConn; /* TCP连接 */
    std::shared_ptr<Request> m_req; /* 请求 */
    std::shared_ptr<Frame> m_frame; /* 数据帧 */
    bool m_isMsgText = false; /* 当前消息是否为文本 */
};
} // namespace ws
} // namespace nsocket
