#pragma once
#include <atomic>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/system_error.hpp>
#include <chrono>
#include <string>

namespace nsocket
{
/**
 * @brief (同步)UDP节点
 */
class SyncUdpNode
{
public:
    /**
     * @brief 构造函数
     * @param bz 数据缓冲区大小(字节)
     */
    SyncUdpNode(size_t bz = 65536);

    virtual ~SyncUdpNode();

    /**
     * @brief 设置非阻塞模式(打开前调用才有效)
     * @param nonBlock 非阻塞模式, true-是, false-否(阻塞模式)
     */
    void setNonBlock(bool nonBlock);

    /**
     * @brief 设置发送缓冲区大小(打开前调用才有效)
     * @param bufferSize 发送缓冲区大小
     */
    void setSendBufferSize(int bufferSize);

    /**
     * @brief 设置接收缓冲区大小(打开前调用才有效)
     * @param bufferSize 接收缓冲区大小
     */
    void setRecvBufferSize(int bufferSize);

    /**
     * @brief 打开
     * @param host 本地地址
     * @param port 本地端口, 为0时自动分配
     * @param broadcast 是否广播, true-是, false-否
     * @param timeout 超时时间
     * @return 错误码
     */
    boost::system::error_code open(const std::string& host, uint16_t port = 0, bool broadcast = false,
                                   const std::chrono::steady_clock::duration& timeout = std::chrono::seconds(3));

    /**
     * @brief 发送数据
     * @param host 远端地址
     * @param port 远端端口
     * @param data 数据
     * @param sentLength [输出]已发送数据长度
     * @return 错误码
     */
    boost::system::error_code send(const std::string& host, uint16_t port, const std::vector<unsigned char>& data,
                                   size_t* sentLength = nullptr);

    /**
     * @brief 接收数据
     * @param host 远端地址
     * @param port 远端端口
     * @param data [输出]数据
     * @param timeout 超时时间
     * @return 错误码
     */
    boost::system::error_code recv(const std::string& host, uint16_t port, std::vector<unsigned char>& data,
                                   const std::chrono::steady_clock::duration& timeout = std::chrono::seconds(3));

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 是否非阻塞模式
     * @return true-非阻塞, false-阻塞
     */
    bool isNonBlock() const;

    /**
     * @brief 获取发送缓冲区大小
     * @return 发送缓冲区大小
     */
    int getSendBufferSize() const;

    /**
     * @brief 获取接收缓冲区大小
     * @return 接收缓冲区大小
     */
    int getRecvBufferSize() const;

private:
    /**
     * @brief 运行(内部实现)
     * @param timeout 超时时间
     * @return true-成功, false-失败(超时) 
     */
    bool runImpl(const std::chrono::steady_clock::duration& timeout);

private:
    boost::asio::io_context m_ioContext; /* IO上下文 */
    boost::asio::ip::udp::socket m_socket; /* 套接字 */
    std::atomic<int> m_nonBlock = {-1}; /* 是否非阻塞: <0-默认, 0-阻塞, 1-非阻塞 */
    std::atomic<int> m_sendBufferSize = {-1}; /* 发送缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::atomic<int> m_recvBufferSize = {-1}; /* 接收缓冲区大小(字节), <=0-默认, >0-指定大小 */
    std::vector<unsigned char> m_recvBuf; /* 接收缓冲区 */
};
} // namespace nsocket
