#pragma once
#include <boost/asio/ip/address.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "tcp_session.h"

namespace nsocket
{
/**
 * @brief TCP新连接回调
 * @param wpSession 会话
 */
using TCP_CONN_NEW_CALLBACK = std::function<void(const std::weak_ptr<TcpSession>& wpSession)>;

/**
 * @brief TCP数据回调
 * @param wpSession 会话
 * @param data 数据
 */
using TCP_CONN_DATA_CALLBACK = std::function<void(const std::weak_ptr<TcpSession>& wpSession, const std::vector<unsigned char>& data)>;

/**
 * @brief TCP连接关闭回调
 * @param sid 会话ID
 * @param point 端点
 * @param code 错误码
 */
using TCP_CONN_CLOSE_CALLBACK =
    std::function<void(int64_t sid, const boost::asio::ip::tcp::endpoint& point, const boost::system::error_code& code)>;

/**
 * @brief TCP服务端(注意: 需要实例化为共享指针否则会报错, 2.停止后需要重新实例化, 不可复用之前的实例)
 */
class TcpServer final : public std::enable_shared_from_this<TcpServer>
{
public:
    /**
     * @brief 构造函数
     * @param host 主机
     * @param port 端口
     * @param reuseAddr 是否允许复用地址(选填)
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    TcpServer(const std::string& host, unsigned int port, bool reuseAddr = true, size_t bz = 1024);

    virtual ~TcpServer() = default;

    /**
     * @brief 设置新连接回调
     * @param onNewCb 新连接回调
     */
    void setNewConnectionCallback(const TCP_CONN_NEW_CALLBACK& onNewCb);

    /**
     * @brief 设置数据回调
     * @param onDataCb 数据回调
     */
    void setConnectionDataCallback(const TCP_CONN_DATA_CALLBACK& onDataCb);

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

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor; /* 接收器 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext; /* TLS上下文 */
#endif
    size_t m_bufferSize; /* 数据接收缓冲区大小 */
    std::unordered_map<int64_t, std::shared_ptr<TcpSession>> m_sessionMap; /* 会话表 */
    TCP_CONN_NEW_CALLBACK m_onNewConnectionCallback; /* 新连接回调 */
    TCP_CONN_DATA_CALLBACK m_onConnectionDataCallback; /* 连接数据回调 */
    TCP_CONN_CLOSE_CALLBACK m_onConnectionCloseCallback; /* 连接关闭回调 */
    std::string m_host; /* 主机 */
};
} // namespace nsocket