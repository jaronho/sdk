#pragma once
#include <atomic>
#include <mutex>
#include <string>

#include "tcp_connection.h"

namespace nsocket
{
/**
 * @brief TCP客户端(注意: 1.需要实例化为共享指针否则会报错, 2.停止后需要重新实例化, 不可复用之前的实例)
 */
class TcpClient final : public std::enable_shared_from_this<TcpClient>
{
public:
    /**
     * @brief 构造函数
     * @param localPort 本地端口, 0表示使用自动分配
     * @param bz 数据缓冲区大小(字节)
     */
    TcpClient(uint16_t localPort = 0, size_t bz = 4096);

    virtual ~TcpClient();

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
     * @brief 设置非阻塞模式(运行前调用才有效)
     * @param nonBlock 非阻塞模式, true-是, false-否(阻塞模式)
     */
    void setNonBlock(bool nonBlock);

    /**
     * @brief 设置发送缓冲区大小(运行前调用才有效)
     * @param bufferSize 发送缓冲区大小
     */
    void setSendBufferSize(int bufferSize);

    /**
     * @brief 设置接收缓冲区大小(运行前调用才有效)
     * @param bufferSize 接收缓冲区大小
     */
    void setRecvBufferSize(int bufferSize);

    /**
     * @brief 设置是否启用Nagle算法(运行前调用才有效)
     * @param enable true-启用, false-关闭
     */
    void setNagleEnable(bool enable);

    /**
     * @brief 运行(进入循环, 阻塞和占用调用线程)
     * @param host 远端地址
     * @param port 远端端口
     * @param sslOn 是否开启SSL, true-是, false-否
     * @param sslWay SSL验证方式, 1-单向, 2-双向
     * @param certFmt (证书/私钥)文件格式, 1-DER, 2-PEM
     * @param certFile 证书文件, 例如: client.crt
     * @param pkFile 私钥文件, 例如; client.key
     * @param pkPwd 私钥文件密码, 例如: 123456
     */
    void run(const std::string& host, uint16_t port, bool sslOn = false, int sslWay = 1, int certFmt = 2, const std::string& certFile = "",
             const std::string& pkFile = "", const std::string& pkPwd = "");

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
     * @brief 是否启用SSL
     * @return true-是, false-否
     */
    bool isEnableSSL();

    /**
     * @brief 是否在运行
     * @param true-是, false-否
     */
    bool isRunning();

    /**
     * @brief 是否非阻塞模式
     * @return true-非阻塞, false-阻塞
     */
    bool isNonBlock();

    /**
     * @brief 获取发送缓冲区大小
     * @return 发送缓冲区大小
     */
    int getSendBufferSize();

    /**
     * @brief 获取接收缓冲区大小
     * @return 接收缓冲区大小
     */
    int getRecvBufferSize();

    /**
     * @brief 获取是否启用Nagle算法
     * @return true-启用, false-不启用
     */
    bool isNagleEnable();

    /**
     * @brief 获取本端端点
     * @return 本端端点
     */
    boost::asio::ip::tcp::endpoint getLocalEndpoint();

    /**
     * @brief 获取远端端点
     * @return 远端端点
     */
    boost::asio::ip::tcp::endpoint getRemoteEndpoint();

private:
    /**
     * @brief 处理连接结果
     * @param code 错误码
     */
    void handleConnect(const boost::system::error_code& code);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    const uint16_t m_localPort; /* 本地端口 */
    const size_t m_bufferSize; /* 缓冲区大小 */
    std::mutex m_mutex;
#if (1 == ENABLE_NSOCKET_OPENSSL)
    std::shared_ptr<boost::asio::ssl::context> m_sslContext = nullptr; /* TLS上下文 */
#endif
    std::shared_ptr<TcpConnection> m_tcpConn = nullptr; /* TCP连接 */
    TCP_CONNECT_CALLBACK m_onConnectCallback = nullptr; /* 连接回调 */
    TCP_DATA_CALLBACK m_onDataCallback = nullptr; /* 数据回调 */
    std::atomic<int> m_nonBlock = {-1}; /* 是否非阻塞: <0-默认, 0-阻塞, 1-非阻塞 */
    std::atomic<int> m_sendBufferSize = {-1}; /* 发送缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_recvBufferSize = {-1}; /* 接收缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_enableNagle = {-1}; /* 是否禁用Nagle算法, <0-默认, 0-禁用, 1-启用 */
    enum class RunStatus
    {
        idle, /* 空闲 */
        running, /* 运行中 */
    };
    std::atomic<RunStatus> m_runStatus = {RunStatus::idle}; /* 运行状态 */
};
} // namespace nsocket
