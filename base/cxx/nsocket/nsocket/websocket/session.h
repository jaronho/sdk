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
     * @return 会话ID
     */
    uint64_t getId() const;

    /**
     * @brief 获取客户端主机
     * @return 客户端主机
     */
    std::string getClientHost() const;

    /**
     * @brief 获取客户端端口
     * @return 客户端端口
     */
    int getClientPort() const;

    /**
     * @brief 获取URI
     * @return URI
     */
    std::string getUri() const;

    /**
     * @brief 发送文本
     * @param text 文本数据
     * @param isFin 是否最后一个帧
     * @param onSendCb 发送回调
     */
    void sendText(const std::string& text, bool isFin = true, const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送二进制
     * @param text 二进制数据
     * @param isFin 是否最后一个帧
     * @param onSendCb 发送回调
     */
    void sendBytes(const std::vector<unsigned char>& bytes, bool isFin = true, const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送ping命令
     * @param onSendCb 发送回调
     */
    void sendPing(const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送pong命令
     * @param onSendCb 发送回调
     */
    void sendPong(const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 发送关闭命令
     * @param code 关闭命令码
     * @param onSendCb 发送回调
     */
    void sendClose(const CloseCode& code = CloseCode::close_normal, const TCP_SEND_CALLBACK& onSendCb = nullptr);

    /**
     * @brief 当前消息是否为文本
     * @return true-文本, false-二进制
     */
    bool isMsgText() const;

protected:
    std::weak_ptr<TcpConnection> m_wpConn; /* TCP连接 */
    std::shared_ptr<Request> m_req; /* 请求 */
    std::shared_ptr<Frame> m_frame; /* 数据帧 */
    bool m_isMsgText = false; /* 当前消息是否为文本 */
};
} // namespace ws
} // namespace nsocket
