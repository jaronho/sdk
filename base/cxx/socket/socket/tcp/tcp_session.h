#pragma once
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>

#include "../core/socket_tcp.h"

namespace nsocket
{
/**
 * @brief TCP接收数据回调
 * @param data 数据
 */
using TCP_RECV_DATA_CALLBACK = std::function<void(const std::vector<unsigned char>& data)>;

/**
 * @brief TCP会话
 */
class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
    /**
     * @brief 构造函数
     * @param socket 套接字
     */
    TcpSession(const std::shared_ptr<SocketTcpBase>& socket);

    ~TcpSession();

    /**
     * @brief 设置连接回调
     * @param onRecvDataCb 接收数据回调
     */
    void setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb);

    /**
     * @brief 设置接收数据回调
     * @param onRecvDataCb 接收数据回调
     */
    void setRecvDataCallback(const TCP_RECV_DATA_CALLBACK& onRecvDataCb);

    /**
     * @brief 连接
     * @param point 端点
     */
    void connect(const boost::asio::ip::tcp::endpoint& point);

#if (1 == ENABLE_SOCKET_OPENSSL)
    /**
     * @brief 握手(启用TLS才需要)
     * @param type 类型, 客户端或服务端
     * @param onHandshakeCb 握手回调
     */
    void handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb);
#endif

    /**
     * @brief 发送数据
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 接收数据(只需要调用一次, 内部递归调用)
     */
    void recv();

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 是否启动用TLS
     * @return true-是, false-否
     */
    bool isEnableTLS() const;

    /**
     * @brief 是否已连接
     * @return true-是, false-否
     */
    bool isConnected() const;

    /**
     * @brief 获取远端端点
     * @return 远端端点
     */
    boost::asio::ip::tcp::endpoint getRemoteEndpoint() const;

private:
    std::shared_ptr<SocketTcpBase> m_socketTcpBase; /* 套接字 */
    bool m_isEnableTLS; /* 是否启用TLS */
    bool m_isConnected; /* 是否已连接上 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
    TCP_CONNECT_CALLBACK m_onConnectCallback; /* 连接回调 */
    TCP_RECV_DATA_CALLBACK m_onRecvDataCallback; /* 接收数据回调 */
};
} // namespace nsocket
