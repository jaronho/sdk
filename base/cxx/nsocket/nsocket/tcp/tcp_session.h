#pragma once
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
 * @brief TCP会话, 注意: 调用close后示例不可再使用
 */
class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
    /**
     * @brief 构造函数
     * @param socket 套接字
     * @param alreadyConnected 是否已经连接上(选填), 服务器中接收到新连接时需要设置该参数为true
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    TcpSession(const std::shared_ptr<SocketTcpBase>& socket, bool alreadyConnected = false, size_t bz = 1024);

    ~TcpSession();

    /**
     * @brief 获取会话ID
     * @return 会话ID
     */
    int64_t getId() const;

    /**
     * @brief 调整数据缓冲区大小
     * @param bz 缓冲区大小(字节)
     */
    void resizeBuffer(size_t bz);

    /**
     * @brief 设置连接回调
     * @param onDataCb 数据回调
     */
    void setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb);

    /**
     * @brief 设置数据回调
     * @param onDataCb 数据回调
     */
    void setDataCallback(const TCP_DATA_CALLBACK& onDataCb);

    /**
     * @brief 连接
     * @param point 端点
     */
    void connect(const boost::asio::ip::tcp::endpoint& point);

#if (1 == ENABLE_NSOCKET_OPENSSL)
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
     * @brief 是否启动用SSL
     * @return true-是, false-否
     */
    bool isEnableSSL() const;

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

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 创建SSL上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param m 方法, 例如: 客户端可以用sslv23_client, 服务端可以用sslv23_server
     * @param certFile 证书文件, 例如: client.crt 或 server.crt
     * @param privateKeyFile 私钥文件, 例如: client.key 或 server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: 123456
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSslContext(boost::asio::ssl::context::method m, const std::string& certFile,
                                                                     const std::string& privateKeyFile,
                                                                     const std::string& privateKeyFilePwd);
#endif

private:
    int64_t m_id; /* ID */
    std::shared_ptr<SocketTcpBase> m_socketTcpBase; /* 套接字 */
    bool m_isEnableSSL; /* 是否启用SSL */
    bool m_isConnected; /* 是否已连接上 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
    TCP_CONNECT_CALLBACK m_onConnectCallback; /* 连接回调 */
    TCP_DATA_CALLBACK m_onDataCallback; /* 数据回调 */
};
} // namespace nsocket