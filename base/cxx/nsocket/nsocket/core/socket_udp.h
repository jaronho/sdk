#pragma once
#include <atomic>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/system_error.hpp>
#include <functional>

namespace nsocket
{
/**
 * @brief UDP套接字打开回调
 * @param code 错误码
 */
using UDP_OPEN_CALLBACK = std::function<void(const boost::system::error_code& code)>;

/**
 * @brief UDP发送回调
 * @param code 错误码
 * @param length 数据长度
 */
using UDP_SEND_CALLBACK = std::function<void(const boost::system::error_code& code, size_t length)>;

/**
 * @brief UDP接收回调
 * @param point 远端端点
 * @param code 错误码
 * @param length 数据长度
 */
using UDP_RECV_CALLBACK =
    std::function<void(const boost::asio::ip::udp::endpoint& point, const boost::system::error_code& code, size_t length)>;

/**
 * @brief UDP套接字基类
 */
class SocketUdpBase
{
public:
    SocketUdpBase() = default;
    virtual ~SocketUdpBase() = default;

    /**
     * @brief 打开套接字
     * @param point 本地端点
     * @param broadcast 是否广播, true-是, false-否
     * @param onOpenCb 打开回调
     */
    virtual void open(const boost::asio::ip::udp::endpoint& point, bool broadcast, const UDP_OPEN_CALLBACK& onOpenCb) = 0;

    /**
     * @brief 发送数据
     * @param point 远端端点
     * @param data 数据
     * @param onSendCb 发送回调
     */
    virtual void send(const boost::asio::ip::udp::endpoint& point, const boost::asio::const_buffer& data,
                      const UDP_SEND_CALLBACK& onSendCb) = 0;

    /**
     * @brief 接收数据
     * @param data [输出]数据
     * @param onRecvCb 接收回调
     */
    virtual void recv(const boost::asio::mutable_buffer& data, const UDP_RECV_CALLBACK& onRecvCb) = 0;

    /**
     * @brief 关闭套接字
     */
    virtual void close() = 0;

    /**
     * @brief 套接字是否已打开
     * @return true-已打开, false-关闭
     */
    virtual bool isOpened() const = 0;

    /**
     * @brief 获取本地端点
     * @return 本地端点
     */
    virtual boost::asio::ip::udp::endpoint getLocalEndpoint() const = 0;

    /**
     * @brief 是否非阻塞模式
     * @return true-非阻塞, false-阻塞
     */
    virtual bool isNonBlock() const = 0;

    /**
     * @brief 设置非阻塞模式
     * @param nonBlock 非阻塞模式, true-是, false-否(阻塞模式)
     * @return true-设置成功, false-设置失败
     */
    virtual bool setNonBlock(bool nonBlock) = 0;

    /**
     * @brief 获取发送缓冲区大小
     * @return 发送缓冲区大小
     */
    virtual size_t getSendBufferSize() const = 0;

    /**
     * @brief 设置发送缓冲区大小
     * @param bufferSize 发送缓冲区大小
     * @return true-设置成功, false-设置失败
     */
    virtual bool setSendBufferSize(size_t bufferSize) = 0;

    /**
     * @brief 获取接收缓冲区大小
     * @return 接收缓冲区大小
     */
    virtual size_t getRecvBufferSize() const = 0;

    /**
     * @brief 设置接收缓冲区大小
     * @param bufferSize 接收缓冲区大小
     * @return true-设置成功, false-设置失败
     */
    virtual bool setRecvBufferSize(size_t bufferSize) = 0;

protected:
    boost::asio::ip::udp::endpoint m_localPoint; /* 本地端点 */
};

/**
 * @brief UDP套接字
 */
class SocketUdp : public SocketUdpBase
{
public:
    SocketUdp(boost::asio::ip::udp::socket socket);

    ~SocketUdp();

    void open(const boost::asio::ip::udp::endpoint& point, bool broadcast, const UDP_OPEN_CALLBACK& onOpenCb) override;

    void send(const boost::asio::ip::udp::endpoint& point, const boost::asio::const_buffer& data,
              const UDP_SEND_CALLBACK& onSendCb) override;

    void recv(const boost::asio::mutable_buffer& data, const UDP_RECV_CALLBACK& onRecvCb) override;

    void close() override;

    bool isOpened() const override;

    boost::asio::ip::udp::endpoint getLocalEndpoint() const override;

    bool isNonBlock() const override;

    bool setNonBlock(bool nonBlock) override;

    size_t getSendBufferSize() const override;

    bool setSendBufferSize(size_t bufferSize) override;

    size_t getRecvBufferSize() const override;

    bool setRecvBufferSize(size_t bufferSize) override;

private:
    boost::asio::ip::udp::socket m_socket;
    boost::asio::ip::udp::endpoint m_remotePoint; /* 远端端点 */
};
} // namespace nsocket