#pragma once
#include <string>

#include "tcp_session.h"

namespace nsocket
{
/**
 * @brief TCP客户端
 */
class TcpClient final : public std::enable_shared_from_this<TcpClient>
{
public:
    TcpClient();

    virtual ~TcpClient() = default;

    /**
     * @brief 设置连接回调
     * @param onConnectCb 连接回调
     */
    void setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb);

    /**
     * @brief 设置接收数据回调
     * @param onRecvDataCb 接收数据回调
     */
    void setRecvDataCallback(const TCP_RECV_DATA_CALLBACK& onRecvDataCb);

    /**
     * @brief 运行(进入循环, 占用调用线程)
     * @param host 服务器
     * @param port 端口
     * @param sslContext TLS上下文(选填), 为空表示不启用TLS
     */
#if (1 == ENABLE_SOCKET_OPENSSL)
    void run(const std::string& host, unsigned int port, const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run(const std::string& host, unsigned int port);
#endif
    /**
     * @brief 发送数据
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否在运行
     * @param true-是, false-否
     */
    bool isRunning() const;

private:
    /**
     * @brief 处理连接结果
     * @param code 错误码
     */
    void handleConnect(const boost::system::error_code& code);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    boost::asio::ip::tcp::resolver::results_type m_endpoints; /* 端点 */
    boost::asio::ip::tcp::resolver::iterator m_endpointIter; /* 当前端点迭代器 */
#if (1 == ENABLE_SOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext; /* TLS上下文 */
#endif
    std::shared_ptr<TcpSession> m_tcpSession; /* TCP会话 */
    TCP_CONNECT_CALLBACK m_onConnectCallback; /* 连接回调 */
    TCP_RECV_DATA_CALLBACK m_onRecvDataCallback; /* 数据接收回调 */
    enum class RunStatus
    {
        RUN_NONE, /* 未开始 */
        RUN_START, /* 开始 */
        RUN_STOP /* 停止 */
    };
    RunStatus m_runStatus; /* 运行状态 */
};
} // namespace nsocket
