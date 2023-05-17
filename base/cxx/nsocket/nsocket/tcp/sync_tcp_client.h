#pragma once
#include <atomic>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>
#include <chrono>
#include <string>

namespace nsocket
{
/**
 * @brief (同步)TCP客户端
 */
class SyncTcpClient
{
public:
    /**
     * @brief 构造函数
     * @param bz 数据缓冲区大小(字节)
     */
    SyncTcpClient(size_t bz = 4096);

    virtual ~SyncTcpClient();

    /**
     * @brief 设置本地端口(连接前调用才有效)
     * @param port 本地端口, 为0时自动分配
     */
    void setLocalPort(uint16_t port);

    /**
     * @brief 连接
     * @param host 远端地址
     * @param port 远端端口
     * @param timeout 超时时间
     * @return 错误码
     */
    boost::system::error_code connect(const std::string& host, uint16_t port,
                                      const std::chrono::steady_clock::duration& timeout = std::chrono::seconds(3));

    /**
     * @brief 发送数据
     * @param data 数据
     * @param sentLength [输出]已发送数据长度
     * @return 错误码
     */
    boost::system::error_code send(const std::vector<unsigned char>& data, size_t* sentLength = nullptr);

    /**
     * @brief 接收数据
     * @param data [输出]数据
     * @param timeout 超时时间
     * @return 错误码
     */
    boost::system::error_code recv(std::vector<unsigned char>& data,
                                   const std::chrono::steady_clock::duration& timeout = std::chrono::seconds(3));

    /**
     * @brief 停止
     */
    void stop();

    /**
     * @brief 获取发送缓冲区大小
     * @return 发送缓冲区大小
     */
    size_t getSendBufferSize() const;

    /**
     * @brief 设置发送缓冲区大小
     * @param bufferSize 发送缓冲区大小
     * @return true-设置成功, false-设置失败
     */
    bool setSendBufferSize(size_t bufferSize);

    /**
     * @brief 获取接收缓冲区大小
     * @return 接收缓冲区大小
     */
    size_t getRecvBufferSize() const;

    /**
     * @brief 设置接收缓冲区大小
     * @param bufferSize 接收缓冲区大小
     * @return true-设置成功, false-设置失败
     */
    bool setRecvBufferSize(size_t bufferSize);

private:
    /**
     * @brief 运行(内部实现)
     * @param timeout 超时时间
     * @return true-成功, false-失败(超时) 
     */
    bool runImpl(const std::chrono::steady_clock::duration& timeout);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    boost::asio::ip::tcp::socket m_socket; /* 套接字 */
    std::atomic<uint16_t> m_localPort = {0}; /* 本地端口 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
};
} // namespace nsocket
