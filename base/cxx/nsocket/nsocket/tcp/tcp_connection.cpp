#include "tcp_connection.h"

#include <chrono>

namespace nsocket
{
static std::atomic<uint64_t> s_timestamp{0}; /* 注意: std::atomic_uint64_t在某些平台下未定义 */
static std::atomic_int s_count{0};

TcpConnection::TcpConnection(const std::shared_ptr<SocketTcpBase>& socket, bool alreadyConnected, size_t bz) : m_socketTcpBase(socket)
{
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    if (ntp == s_timestamp)
    {
        ++s_count;
    }
    else
    {
        s_count = 0;
        s_timestamp = ntp;
    }
    m_id = (s_timestamp << 12) + (s_count & 0xFFF);
#if (1 == ENABLE_NSOCKET_OPENSSL)
    m_isEnableSSL = (std::dynamic_pointer_cast<SocketTls>(m_socketTcpBase) ? true : false);
#else
    m_isEnableSSL = false;
#endif
    m_isConnected = alreadyConnected;
    resizeBuffer(bz);
}

TcpConnection::~TcpConnection()
{
    close();
}

uint64_t TcpConnection::getId() const
{
    return m_id;
}

size_t TcpConnection::getBufferSize() const
{
    return m_recvBuf.size();
}

void TcpConnection::resizeBuffer(size_t bz)
{
    m_recvBuf.resize(bz > 128 ? bz : 128);
}

void TcpConnection::setConnectCallback(const TCP_CONNECT_CALLBACK& onConnectCb)
{
    m_onConnectCallback = onConnectCb;
}

void TcpConnection::setDataCallback(const TCP_DATA_CALLBACK& onDataCb)
{
    m_onDataCallback = onDataCb;
}

void TcpConnection::setLocalPort(uint32_t port)
{
    if (m_socketTcpBase)
    {
        m_socketTcpBase->setLocalPort(port);
    }
}

void TcpConnection::connect(const boost::asio::ip::tcp::endpoint& point, bool async)
{
    if (m_socketTcpBase)
    {
        const std::weak_ptr<TcpConnection> wpSelf = shared_from_this();
        m_socketTcpBase->connect(
            point,
            [wpSelf](const boost::system::error_code& code) {
                const auto self = wpSelf.lock();
                if (self)
                {
                    if (code) /* 连接失败 */
                    {
                        self->closeImpl();
                        if (self->m_onConnectCallback)
                        {
                            self->m_onConnectCallback(code);
                        }
                    }
                    else /* 连接成功 */
                    {
                        if (self->m_isEnableSSL) /* TLS, 需要握手 */
                        {
                            if (self->m_onConnectCallback)
                            {
                                self->m_onConnectCallback(code);
                            }
                        }
                        else /* TCP, 成功后开始接收数据 */
                        {
                            self->m_isConnected = true;
                            if (self->m_onConnectCallback)
                            {
                                self->m_onConnectCallback(code);
                            }
                            self->recv();
                        }
                    }
                }
            },
            async);
    }
    else if (m_onConnectCallback)
    {
        m_onConnectCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
    }
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
void TcpConnection::handshake(boost::asio::ssl::stream_base::handshake_type type, const TLS_HANDSHAKE_CALLBACK& onHandshakeCb, bool async)
{
    std::shared_ptr<SocketTls> tlsPtr = std::dynamic_pointer_cast<SocketTls>(m_socketTcpBase);
    if (tlsPtr)
    {
        const std::weak_ptr<TcpConnection> wpSelf = shared_from_this();
        tlsPtr->handshake(
            type,
            [wpSelf, onHandshakeCb](const boost::system::error_code& code) {
                const auto self = wpSelf.lock();
                if (self)
                {
                    if (code) /* 握手失败 */
                    {
                        self->closeImpl();
                        if (onHandshakeCb)
                        {
                            onHandshakeCb(code);
                        }
                    }
                    else /* 握手成功 */
                    {
                        self->m_isConnected = true;
                        if (onHandshakeCb)
                        {
                            onHandshakeCb(code);
                        }
                        self->recv();
                    }
                }
            },
            async);
    }
    else
    {
        closeImpl();
        if (onHandshakeCb)
        {
            onHandshakeCb(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
        }
    }
}
#endif

void TcpConnection::send(const std::vector<unsigned char>& data, const TCP_SEND_CALLBACK& onSendCb)
{
    if (m_socketTcpBase && m_isConnected)
    {
        if (data.empty())
        {
            if (onSendCb)
            {
                onSendCb(boost::system::errc::make_error_code(boost::system::errc::no_message_available), 0);
            }
        }
        else
        {
            boost::system::error_code code;
            size_t sentLength = 0;
            while (sentLength < data.size()) /* 循环发送所有数据 */
            {
                m_socketTcpBase->send(boost::asio::buffer(data.data() + sentLength, data.size() - sentLength),
                                      [&code, &sentLength](const boost::system::error_code& ec, size_t length) {
                                          code = ec;
                                          sentLength += length;
                                      });
                if (code) /* 发送失败 */
                {
                    break;
                }
            }
            if (onSendCb)
            {
                onSendCb(code, sentLength);
            }
        }
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void TcpConnection::TcpConnection::recv()
{
    if (m_socketTcpBase)
    {
        const std::weak_ptr<TcpConnection> wpSelf = shared_from_this();
        m_socketTcpBase->recv(boost::asio::buffer(m_recvBuf), [wpSelf](const boost::system::error_code& code, size_t length) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 接收失败 */
                {
                    bool connectedFlag = self->m_isConnected;
                    self->closeImpl();
                    if (connectedFlag && self->m_onConnectCallback)
                    {
                        self->m_onConnectCallback(code);
                    }
                }
                else /* 接收成功 */
                {
                    if (self->m_onDataCallback)
                    {
                        std::vector<unsigned char> data;
                        const unsigned char* rawData = (const unsigned char*)self->m_recvBuf.data();
                        if (rawData && length > 0)
                        {
                            data.insert(data.end(), rawData, rawData + length);
                        }
                        self->m_onDataCallback(data);
                    }
                    self->recv(); /* 继续接收 */
                }
            }
        });
    }
    else
    {
        closeImpl();
        if (m_onConnectCallback)
        {
            m_onConnectCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
        }
    }
}

void TcpConnection::closeImpl()
{
    m_isConnected = false;
    if (m_socketTcpBase)
    {
        m_socketTcpBase->close();
    }
}

void TcpConnection::close()
{
    bool connectedFlag = m_isConnected;
    closeImpl();
    if (connectedFlag && m_onConnectCallback)
    {
        m_onConnectCallback(boost::system::errc::make_error_code(boost::system::errc::interrupted));
    }
}

bool TcpConnection::isEnableSSL() const
{
    return m_isEnableSSL;
}

bool TcpConnection::isConnected() const
{
    return m_isConnected;
}

boost::asio::ip::tcp::endpoint TcpConnection::getLocalEndpoint() const
{
    if (m_socketTcpBase)
    {
        return m_socketTcpBase->getLocalEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

boost::asio::ip::tcp::endpoint TcpConnection::getRemoteEndpoint() const
{
    if (m_socketTcpBase)
    {
        return m_socketTcpBase->getRemoteEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

bool TcpConnection::setNonBlock(bool nonBlock)
{
    if (m_socketTcpBase)
    {
        return m_socketTcpBase->setNonBlock(nonBlock);
    }
    return false;
}

#if (1 == ENABLE_NSOCKET_OPENSSL)
std::shared_ptr<boost::asio::ssl::context> TcpConnection::makeSsl1WayContextClient(boost::asio::ssl::context::method m,
                                                                                   bool allowSelfSigned)
{
    auto sslContext = std::make_shared<boost::asio::ssl::context>(m);
    sslContext->set_verify_mode(boost::asio::ssl::verify_peer);
    sslContext->set_verify_callback([allowSelfSigned](bool preverified, boost::asio::ssl::verify_context& ctx) -> bool {
        X509_STORE_CTX* cts = ctx.native_handle();
        X509* cert = X509_STORE_CTX_get_current_cert(cts);
        int errorCode = X509_STORE_CTX_get_error(cts);
        switch (errorCode)
        {
        case X509_V_OK:
            preverified = true;
            break;
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            if (allowSelfSigned)
            {
                preverified = true;
            }
            break;
        default:
            break;
        }
        return preverified;
    });
    return sslContext;
}

std::shared_ptr<boost::asio::ssl::context>
TcpConnection::makeSsl1WayContextServer(boost::asio::ssl::context::method m, boost::asio::ssl::context::file_format fileFmt,
                                        const std::string& certFile, const std::string& privateKeyFile,
                                        const std::string& privateKeyFilePwd, bool allowSelfSigned)
{
    if (certFile.empty() || privateKeyFile.empty())
    {
        return nullptr;
    }
    auto sslContext = std::make_shared<boost::asio::ssl::context>(m);
    sslContext->use_certificate_file(certFile, fileFmt);
    /* 注意: 需要先调用`set_password_callback`再调用`use_private_key_file`自动填充密码, 否则若有密码时会提示需要输入密码 */
    sslContext->set_password_callback(
        [privateKeyFilePwd](size_t maxLength, boost::asio::ssl::context::password_purpose passwordPurpose) -> std::string {
            return privateKeyFilePwd;
        });
    sslContext->use_private_key_file(privateKeyFile, fileFmt);
    sslContext->set_verify_mode(boost::asio::ssl::verify_peer);
    sslContext->set_verify_callback([allowSelfSigned](bool preverified, boost::asio::ssl::verify_context& ctx) -> bool {
        X509_STORE_CTX* cts = ctx.native_handle();
        X509* cert = X509_STORE_CTX_get_current_cert(cts);
        int errorCode = X509_STORE_CTX_get_error(cts);
        switch (errorCode)
        {
        case X509_V_OK:
            preverified = true;
            break;
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            if (allowSelfSigned)
            {
                preverified = true;
            }
            break;
        default:
            break;
        }
        return preverified;
    });
    return sslContext;
}

std::shared_ptr<boost::asio::ssl::context> TcpConnection::makeSsl2WayContext(boost::asio::ssl::context::method m,
                                                                             boost::asio::ssl::context::file_format fileFmt,
                                                                             const std::string& certFile, const std::string& privateKeyFile,
                                                                             const std::string& privateKeyFilePwd, bool allowSelfSigned)
{
    if (certFile.empty() || privateKeyFile.empty())
    {
        return nullptr;
    }
    auto sslContext = std::make_shared<boost::asio::ssl::context>(m);
    sslContext->use_certificate_file(certFile, fileFmt);
    /* 注意: 需要先调用`set_password_callback`再调用`use_private_key_file`自动填充密码, 否则若有密码时会提示需要输入密码 */
    sslContext->set_password_callback(
        [privateKeyFilePwd](size_t maxLength, boost::asio::ssl::context::password_purpose passwordPurpose) -> std::string {
            return privateKeyFilePwd;
        });
    sslContext->use_private_key_file(privateKeyFile, fileFmt);
    sslContext->set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::verify_fail_if_no_peer_cert
                                | boost::asio::ssl::verify_client_once); /* 配置启用双向认证 */
    sslContext->set_verify_callback([allowSelfSigned](bool preverified, boost::asio::ssl::verify_context& ctx) -> bool {
        X509_STORE_CTX* cts = ctx.native_handle();
        X509* cert = X509_STORE_CTX_get_current_cert(cts);
        int errorCode = X509_STORE_CTX_get_error(cts);
        switch (errorCode)
        {
        case X509_V_OK:
            preverified = true;
            break;
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            if (allowSelfSigned)
            {
                preverified = true;
            }
            break;
        default:
            break;
        }
        return preverified;
    });
    return sslContext;
}
#endif
} // namespace nsocket
