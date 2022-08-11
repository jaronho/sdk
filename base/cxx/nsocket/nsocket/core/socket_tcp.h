#pragma once
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#if (1 == ENABLE_NSOCKET_OPENSSL)
#include <boost/asio/ssl.hpp>
#endif
#include <boost/system/system_error.hpp>
#include <functional>

/* 
 * 关于同步/异步发送数据说明:
 * 这里使用同步方式, asio异步发送复杂的地方在于, 不能连续调用异步发送接口async_write, 
 * 因为async_write内部是不断调用async_write_some, 直到所有的数据发送完成为止. 由于async_write
 * 调用之后就直接返回了, 如果第一次调用async_write发送一个较大的包时, 马上又再调用async_write
 * 发送一个很小的包时, 有可能这时第一次的async_write还在循环调用async_write_some发送, 而第二次
 * 的async_write要发送的数据很小, 一下子就发出去了, 这使得第一次发送的数据和第二次发送的数据交
 * 织在一起了, 导致发送乱序的问题. 解决这个问题的方法就是在第一次发送完成之后再发送第二次的数据.
 * 具体的做法是用一个发送缓冲区, 在异步发送完成之后从缓冲区再取下一个数据包发送.
 * 
 * 但是还有一个问题需要注意就是这个缓冲区是没有加限制的, 如果接收端收到数据之后阻塞处理, 而发送
 * 又很快的话, 就会导致发送队列的内存快速增长甚至内存爆掉, 解决办法有两个:
 * (1)发慢一点, 并且保证接收端不会长时间阻塞socket.
 * (2)控制发送队列的上限.
 * 第一种方法对实际应用的约束性较强, 实际可操作性不高. 第二种方法需要控制队列上限, 不可避免的要加锁, 
 * 这样就丧失了单线程异步发送的性能优势. 所以建议用同步发送接口来发送数据, 一来不用发送队列, 自然也
 * 不会有内存暴涨的问题, 二来也不会有复杂的循环发送过程, 而且还可以通过线程池来提高发送效率.
 *
 * 总结: 
 * (1)不要连续发起异步发送, 要等上次发送完成之后再发起下一个异步发送.
 * (2)要考虑异步发送的发送队列内存可能会暴涨的问题.
 * (3)相比复杂的异步发送, 同步发送简单可靠, 推荐优先使用同步发送接口.
 */

namespace nsocket
{
/**
 * @brief TCP连接回调
 * @param code 错误码
 */
using TCP_CONNECT_CALLBACK = std::function<void(const boost::system::error_code& code)>;

/**
 * @brief TCP发送回调
 * @param code 错误码
 * @param length 数据长度
 */
using TCP_SEND_CALLBACK = std::function<void(const boost::system::error_code& code, size_t length)>;

/**
 * @brief TCP接收回调
 * @param code 错误码
 * @param length 数据长度
 */
using TCP_RECV_CALLBACK = std::function<void(const boost::system::error_code& code, size_t length)>;

/**
 * @brief TLS握手回调
 * @param code 错误码
 */
using TLS_HANDSHAKE_CALLBACK = std::function<void(const boost::system::error_code& code)>;

/**
 * @brief TCP套接字基类
 */
class SocketTcpBase
{
public:
    SocketTcpBase() = default;
    virtual ~SocketTcpBase() = default;
    virtual void connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async = true) = 0;
    virtual void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb) = 0;
    virtual void recv(const boost::asio::mutable_buffer& data, const TCP_RECV_CALLBACK& onRecvCb) = 0;
    virtual void bind(const boost::asio::ip::tcp::endpoint& point, boost::system::error_code& code) = 0;
    virtual void close() = 0;
    virtual bool isOpened() const = 0;
    virtual boost::asio::ip::tcp::endpoint getRemoteEndpoint() const = 0;
    virtual bool isNonBlock() const = 0;
    virtual bool setNonBlock(bool nonBlock) = 0;
    virtual size_t getSendBufferSize() const = 0;
    virtual bool setSendBufferSize(size_t bufferSize) = 0;
    virtual size_t getRecvBufferSize() const = 0;
    virtual bool setRecvBufferSize(size_t bufferSize) = 0;
};

/**
 * @brief TCP套接字
 */
class SocketTcp : public SocketTcpBase
{
public:
    SocketTcp(boost::asio::ip::tcp::socket socket);

    virtual ~SocketTcp() = default;

    void connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async = true) override;

    void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb) override;

    void recv(const boost::asio::mutable_buffer& data, const TCP_RECV_CALLBACK& onRecvCb) override;

    void bind(const boost::asio::ip::tcp::endpoint& point, boost::system::error_code& code) override;

    void close() override;

    bool isOpened() const override;

    boost::asio::ip::tcp::endpoint getRemoteEndpoint() const override;

    bool isNonBlock() const override;

    bool setNonBlock(bool nonBlock) override;

    size_t getSendBufferSize() const override;

    bool setSendBufferSize(size_t bufferSize) override;

    size_t getRecvBufferSize() const override;

    bool setRecvBufferSize(size_t bufferSize) override;

private:
    boost::asio::ip::tcp::socket m_socket;
};

#if (1 == ENABLE_NSOCKET_OPENSSL)
/**
 * @brief TLS套接字(安全的TCP)
 */
class SocketTls : public SocketTcpBase
{
public:
    SocketTls(boost::asio::ip::tcp::socket socket, boost::asio::ssl::context& sslContext);

    virtual ~SocketTls() = default;

    void connect(const boost::asio::ip::tcp::endpoint& point, const TCP_CONNECT_CALLBACK& onConnectCb, bool async = true) override;

    void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb) override;

    void recv(const boost::asio::mutable_buffer& data, const TCP_RECV_CALLBACK& onRecvCb) override;

    void bind(const boost::asio::ip::tcp::endpoint& host, boost::system::error_code& code) override;

    void close() override;

    bool isOpened() const override;

    boost::asio::ip::tcp::endpoint getRemoteEndpoint() const override;

    bool isNonBlock() const override;

    bool setNonBlock(bool nonBlock) override;

    size_t getSendBufferSize() const override;

    bool setSendBufferSize(size_t bufferSize) override;

    size_t getRecvBufferSize() const override;

    bool setRecvBufferSize(size_t bufferSize) override;

    void handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async = true);

private:
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_sslStream;
};
#endif
} // namespace nsocket
