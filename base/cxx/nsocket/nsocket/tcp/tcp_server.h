#pragma once
#include <boost/asio/ip/address.hpp>
#include <memory>
#include <mutex>
#include <set>

#include "tcp_session.h"

namespace nsocket
{
/**
 * @brief TCP连接发送回调
 * @param point 端点
 * @param code 错误码
 * @param length 数据长度
 */
using TCP_CONN_SEND_CALLBACK =
    std::function<void(const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code, std::size_t length)>;

/**
 * @brief TCP连接发送句柄
 * @param data 数据
 * @param onSendCb 发送回调
 */
using TCP_CONN_SEND_HANDLER = std::function<void(const std::vector<unsigned char>& data, const TCP_CONN_SEND_CALLBACK& onSendCb)>;

/**
 * @brief TCP新连接回调
 * @param point 端点
 * @param sendHandler 发送句柄
 */
using TCP_CONN_NEW_CALLBACK = std::function<void(const boost::asio::ip::tcp::endpoint& point, const TCP_CONN_SEND_HANDLER& sendHandler)>;

/**
 * @brief TCP接收数据回调
 * @param point 端点
 * @param data 数据
 * @param sendHandler 发送句柄
 */
using TCP_CONN_RECV_DATA_CALLBACK = std::function<void(const boost::asio::ip::tcp::endpoint& point, const std::vector<unsigned char>& data,
                                                       const TCP_CONN_SEND_HANDLER& sendHandler)>;

/**
 * @brief TCP连接关闭回调
 * @param point 端点
 * @param code 错误码
 */
using TCP_CONN_CLOSE_CALLBACK = std::function<void(const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

/**
 * @brief TCP服务端(注意: 需要实例化为共享指针否则会报错)
 */
class TcpServer final : public std::enable_shared_from_this<TcpServer>
{
public:
    /**
     * @brief 构造函数
     * @param host 主机
     * @param port 端口
     */
    TcpServer(const std::string& host, unsigned int port);

    virtual ~TcpServer() = default;

    /**
     * @brief 设置新连接回调
     * @param onNewCb 新连接回调
     */
    void setNewConnectionCallback(const TCP_CONN_NEW_CALLBACK& onNewCb);

    /**
     * @brief 设置接收数据回调
     * @param onRecvDataCb 接收数据回调
     */
    void setRecvConnectionDataCallback(const TCP_CONN_RECV_DATA_CALLBACK& onRecvDataCb);

    /**
     * @brief 设置连接关闭回调
     * @param onCloseCb 连接关闭回调
     */
    void setConnectionCloseCallback(const TCP_CONN_CLOSE_CALLBACK& onCloseCb);

    /**
     * @brief 运行(进入循环, 占用调用线程)
     * @param sslContext TLS上下文(选填), 为空表示不启用TLS
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr);
#else
    void run();
#endif
    /**
     * @brief 停止
     */
    void stop();

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 获取SSL上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param certFile 证书文件, 例如: server.crt
     * @param privateKeyFile 私钥文件, 例如: server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: 123456
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> getSslContext(const std::string& certFile, const std::string& privateKeyFile,
                                                                    const std::string& privateKeyFilePwd);
#endif

private:
    /**
     * @brief 接收客户端连接请求
     */
    void doAccept();

    /**
     * @brief 发送数据到客户端
     * @param connection 客户端连接
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void doSend(const std::shared_ptr<TcpSession>& connection, const std::vector<unsigned char>& data,
                const TCP_CONN_SEND_CALLBACK& onSendCb);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor; /* 接收器 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext; /* TLS上下文 */
#endif
    TCP_CONN_NEW_CALLBACK m_onNewConnectionCallback; /* 新连接回调 */
    TCP_CONN_RECV_DATA_CALLBACK m_onRecvConnectionDataCallback; /* 收到数据回调 */
    TCP_CONN_CLOSE_CALLBACK m_onConnectionCloseCallback; /* 连接关闭回调 */
};
} // namespace nsocket
