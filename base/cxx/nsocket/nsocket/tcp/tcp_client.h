#pragma once
#include <atomic>
#include <string>

#include "tcp_connection.h"

namespace nsocket
{
/**
 * @brief TCP客户端(注意: 1.需要实例化为共享指针否则会报错, 2.连接断开后需要重新实例化, 不可复用之前的实例)
 */
class TcpClient final : public std::enable_shared_from_this<TcpClient>
{
public:
    /**
     * @brief 构造函数
     * @param bz 数据缓冲区大小(字节, 选填)
     */
    TcpClient(size_t bz = 4096);

    virtual ~TcpClient() = default;

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
     * @brief 设置本地端口(运行前调用才有效)
     * @param port 本地端口
     */
    void setLocalPort(uint32_t port);

    /**
     * @brief 运行(进入循环, 阻塞和占用调用线程)
     * @param host 服务器
     * @param port 端口
     * @param sslContext TLS上下文(选填), 为空表示不启用TLS
     * @param async 是否异步连接(选填), 默认异步
     */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    void run(const std::string& host, unsigned int port, const std::shared_ptr<boost::asio::ssl::context>& sslContext = nullptr,
             bool async = true);
#else
    void run(const std::string& host, unsigned int port, bool async = true);
#endif

    /**
     * @brief 发送数据(同步)
     * @param data 数据
     * @param sentLength [输出]已发送数据的长度
     * @param 错误码
     */
    boost::system::error_code send(const std::vector<unsigned char>& data, size_t& sentLength);

    /**
     * @brief 发送数据(异步)
     * @param data 数据
     * @param onSendCb 发送回调
     */
    void sendAsync(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb);

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 是否在运行
     * @param true-是, false-否
     */
    bool isRunning() const;

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

#if (1 == ENABLE_NSOCKET_OPENSSL)
    /**
     * @brief 获取SSL(单向验证)上下文
     * @param fileFmt 文件格式
     * @param certFile 证书文件, 例如: ca.crt
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> getSsl1WayContext(boost::asio::ssl::context::file_format fileFmt,
                                                                        const std::string& certFile);

    /**
     * @brief 获取SSL(双向验证)上下文(当证书文件或私钥文件为空时返回空)
     * @param fileFmt 文件格式
     * @param certFile 证书文件, 例如: client.crt
     * @param privateKeyFile 私钥文件, 例如: client.key
     * @param privateKeyFilePwd 私钥文件密码, 例如: qq123456
     * @return SSL上下文
     */
    static std::shared_ptr<boost::asio::ssl::context> getSsl2WayContext(boost::asio::ssl::context::file_format fileFmt,
                                                                        const std::string& certFile, const std::string& privateKeyFile,
                                                                        const std::string& privateKeyFilePwd);
#endif

private:
    /**
     * @brief 处理连接结果
     * @param code 错误码
     * @param async 是否异步
     */
    void handleConnect(const boost::system::error_code& code, bool async);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    uint32_t m_localPort; /* 本地端口 */
    boost::asio::ip::tcp::resolver::results_type m_endpoints; /* 远端端点 */
    boost::asio::ip::tcp::resolver::iterator m_endpointIter; /* 远端端点迭代器 */
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext; /* TLS上下文 */
#endif
    std::shared_ptr<TcpConnection> m_tcpConn; /* TCP连接 */
    size_t m_bufferSize; /* 数据接收缓冲区大小 */
    TCP_CONNECT_CALLBACK m_onConnectCallback; /* 连接回调 */
    TCP_DATA_CALLBACK m_onDataCallback; /* 数据回调 */
    enum class RunStatus
    {
        none, /* 未开始 */
        start, /* 开始 */
        stop /* 停止 */
    };
    std::atomic<RunStatus> m_runStatus = {RunStatus::none}; /* 运行状态 */
};
} // namespace nsocket
