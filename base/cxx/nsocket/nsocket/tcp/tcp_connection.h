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
 * @brief TCP连接, 注意: 调用close后示例不可再使用
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @brief 构造函数
     * @param socket 套接字
     * @param alreadyConnected 是否已经连接上(选填), 服务器中接收到新连接时需要设置该参数为true
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    TcpConnection(const std::shared_ptr<SocketTcpBase>& socket, bool alreadyConnected = false, size_t bz = 1024);

    ~TcpConnection();

    /**
     * @brief 获取连接ID
     * @return 连接ID
     */
    int64_t getId() const;

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
     * @param async 是否异步连接(选填), 默认异步
     */
    void connect(const boost::asio::ip::tcp::endpoint& point, bool async = true);

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 握手(启用TLS才需要)
     * @param type 类型, 客户端或服务端
     * @param onHandshakeCb 握手回调
     * @param async 是否异步握手(选填), 默认异步
     */
    void handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async = true);
#endif

    /**
     * @brief 发送数据(同步)
     * @param data 数据
     * @param length [输出]已发送数据的长度
     * @param 错误码
     */
    boost::system::error_code send(const std::vector<unsigned char>& data, size_t& length);

    /**
     * @brief 发送数据(异步)
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void sendAsync(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

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

    /**
     * @brief 设置非阻塞(说明: 需要连接成功后才调用)
     * @param nonBlock true-非阻塞, false-阻塞
     * @return true-设置成功, false-设置失败
     */
    bool setNonBlock(bool nonBlock);

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 创建客户端SSL(单向验证)上下文
     * @param m 方法, 例如: 可以用sslv23_client
     * @param caFile 根证书文件, 例如: ca.crt
     * @param allowSelfSigned 是否允许自签证书通过(选填), 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSsl1WayContextClient(boost::asio::ssl::context::method m,
                                                                               const std::string& caFile, bool allowSelfSigned = true);

    /**
     * @brief 创建服务端SSL(单向验证)上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param m 方法, 例如: 可以用sslv23_server
     * @param certFile 证书文件, 例如: client.crt 或 server.crt
     * @param privateKeyFile 私钥文件, 例如: client.key 或 server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param allowSelfSigned 是否允许自签证书通过(选填), 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context>
    makeSsl1WayContextServer(boost::asio::ssl::context::method m, const std::string& certFile, const std::string& privateKeyFile,
                             const std::string& privateKeyFilePwd, bool allowSelfSigned = true);

    /**
     * @brief 创建SSL(双向验证)上下文(当证书文件,私钥文件,私钥文件密码都为空时返回空)
     * @param m 方法, 例如: 客户端可以用sslv23_client, 服务端可以用sslv23_server
     * @param certFile 证书文件, 例如: client.crt 或 server.crt
     * @param privateKeyFile 私钥文件, 例如: client.key 或 server.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @param allowSelfSigned 是否允许自签证书通过(选填), 默认允许
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> makeSsl2WayContext(boost::asio::ssl::context::method m, const std::string& certFile,
                                                                         const std::string& privateKeyFile,
                                                                         const std::string& privateKeyFilePwd, bool allowSelfSigned = true);
#endif

private:
    /**
     * @brief 发送数据(同步)内部实现
     * @param data 数据
     * @param totalSentLength [输出]总的已发送数据的长度
     * @param 错误码
     */
    boost::system::error_code sendImpl(const std::vector<unsigned char>& data, size_t& totalSentLength);

    /**
     * @brief 发送数据(异步)内部实现
     * @param sentLength 总的已发送数据的长度
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void sendAsyncImpl(size_t totalSentLength, const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 接收数据(只需要调用一次, 内部递归调用)
     */
    void recv();

private:
    int64_t m_id; /* ID */
    std::shared_ptr<SocketTcpBase> m_socketTcpBase; /* 套接字 */
    std::atomic_bool m_isEnableSSL = {false}; /* 是否启用SSL */
    std::atomic_bool m_isConnected = {false}; /* 是否已连接上 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
    TCP_CONNECT_CALLBACK m_onConnectCallback; /* 连接回调 */
    TCP_DATA_CALLBACK m_onDataCallback; /* 数据回调 */
};
} // namespace nsocket
