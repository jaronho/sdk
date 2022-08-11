#pragma once
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#if (1 == ENABLE_NSOCKET_OPENSSL)
#include <boost/asio/ssl.hpp>
#endif
#include <boost/system/system_error.hpp>
#include <functional>

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
    virtual void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb, bool async = true) = 0;
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

    void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb, bool async = true) override;

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

    void send(const boost::asio::const_buffer& data, const TCP_SEND_CALLBACK& onSendCb, bool async = true) override;

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
