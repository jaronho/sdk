#pragma once
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <vector>

#include "../core/socket_tcp.h"

namespace nsocket
{
/**
 * @brief TCP数据回调
 * @param data 数据
 */
using TCP_DATA_CALLBACK = std::function<void(const std::vector<unsigned char>& data)>;

/**
 * @brief TCP连接, 注意: 调用close后实例不可再使用, 需要重新创建
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @brief 构造函数
     * @param socket 套接字
     * @param alreadyConnected 是否已经连接上, 服务器中接收到新连接时需要设置该参数为true
     * @param bz 数据缓冲区大小(字节)
     */
    TcpConnection(const std::shared_ptr<SocketTcpBase>& socket, bool alreadyConnected = false, size_t bz = 4096);

    ~TcpConnection();

    /**
     * @brief 获取连接ID
     * @return 连接ID
     */
    uint64_t getId() const;

    /**
     * @brief 获取数据缓冲区大小
     * @return 缓冲区大小(字节)
     */
    size_t getBufferSize() const;

    /**
     * @brief 调整数据缓冲区大小
     * @param bz 缓冲区大小(字节)
     */
    void resizeBuffer(size_t bz);

    /**
     * @brief 设置连接回调
     * @param onConnectCb 连接回调
     */
    void setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb);

    /**
     * @brief 设置数据回调
     * @param onDataCb 数据回调
     */
    void setDataCallback(const TCP_DATA_CALLBACK& onDataCb);

    /**
     * @brief 设置本地端口(连接前调用才有效)
     * @param port 本地端口
     */
    void setLocalPort(uint16_t port);

    /**
     * @brief 连接
     * @param point 远端端点
     * @param async 是否异步连接, 默认异步
     */
    void connect(const boost::asio::ip::tcp::endpoint& point, bool async = true);

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 握手(启用TLS才需要)
     * @param type 类型, 客户端或服务端
     * @param onHandshakeCb 握手回调
     * @param async 是否异步握手, 默认异步
     */
    void handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async = true);
#endif

    /**
     * @brief 发送数据(同步)
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 是否启用SSL
     * @return true-是, false-否
     */
    bool isEnableSSL() const;

    /**
     * @brief 是否已连接
     * @return true-是, false-否
     */
    bool isConnected() const;

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::tcp::endpoint getLocalEndpoint() const;

    /**
     * @brief 获取远端端点
     * @return 远端端点
     */
    boost::asio::ip::tcp::endpoint getRemoteEndpoint() const;

    /**
     * @brief 设置非阻塞(说明: 需要连接成功后才调用)
     * @param nonBlock true-非阻塞, false-阻塞
     * @return true-设置成功, false-设置失败
     */
    bool setNonBlock(bool nonBlock);

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 创建客户端SSL(单向验证)上下文(不需要证书文件)
     * @param m 方法, 例如: 可以用sslv23_client
     * @param allowSelfSigned 是否允许自签证书通过, 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSsl1WayContextClient(boost::asio::ssl::context::method m,
                                                                               bool allowSelfSigned = true);

    /**
     * @brief 创建服务端SSL(单向验证)上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param m 方法, 例如: 可以用sslv23_server
     * @param certFmt 文件格式
     * @param certFile 证书文件, 例如: client.crt, server.crt
     * @param pkFile 私钥文件, 例如: client.key, server.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @param allowSelfSigned 是否允许自签证书通过, 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSsl1WayContextServer(boost::asio::ssl::context::method m,
                                                                               boost::asio::ssl::context::file_format certFmt,
                                                                               const std::string& certFile, const std::string& pkFile,
                                                                               const std::string& pkPwd, bool allowSelfSigned = true);

    /**
     * @brief 创建SSL(双向验证)上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param m 方法, 例如: 客户端可以用sslv23_client, 服务端可以用sslv23_server
     * @param certFmt 文件格式
     * @param certFile 证书文件, 例如: client.crt, server.crt
     * @param pkFile 私钥文件, 例如: client.key, server.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     * @param allowSelfSigned 是否允许自签证书通过, 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSsl2WayContext(boost::asio::ssl::context::method m,
                                                                         boost::asio::ssl::context::file_format certFmt,
                                                                         const std::string& certFile, const std::string& pkFile,
                                                                         const std::string& pkPwd, bool allowSelfSigned = true);
#endif

private:
    /**
     * @brief 接收数据(只需要调用一次, 内部递归调用)
     */
    void recv();

    /**
     * @brief 关闭(内部实现)
     */
    void closeImpl();

private:
    uint64_t m_id = 0; /* ID */
    std::shared_ptr<SocketTcpBase> m_socketTcpBase = nullptr; /* 套接字 */
    std::atomic_bool m_isEnableSSL = {false}; /* 是否启用SSL */
    std::atomic_bool m_isConnected = {false}; /* 是否已连接上 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
    TCP_CONNECT_CALLBACK m_onConnectCallback = nullptr; /* 连接回调 */
    TCP_DATA_CALLBACK m_onDataCallback = nullptr; /* 数据回调 */
};
} // namespace nsocket
