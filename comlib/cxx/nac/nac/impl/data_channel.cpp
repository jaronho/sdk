#include "data_channel.h"

namespace nac
{
std::weak_ptr<threading::Executor> DataChannel::getPktExecutor()
{
    return m_pktExecutor;
}

bool DataChannel::connect(const std::string& address, unsigned short port, bool filePEM, const std::string& certFile,
                          const std::string& privateKeyFile, const std::string& privateKeyFilePwd)
{
    try
    {
        if (m_tcpClient)
        {
            WARN_LOG(m_logger, "连接失败: 重复连接.");
            return false;
        }
        INFO_LOG(m_logger, "连接服务器: {}:{}.", address, port);
        const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
        const std::weak_ptr<threading::Executor> wpPktExecutor = m_pktExecutor;
        m_tcpClient = std::make_shared<nsocket::TcpClient>();
        m_tcpClient->setConnectCallback([wpSelf, wpPktExecutor, logger = m_logger](const boost::system::error_code& code) {
            const auto pktExecutor = wpPktExecutor.lock();
            if (pktExecutor)
            {
                auto fn = [wpSelf, code, logger]() {
                    const auto self = wpSelf.lock();
                    if (self)
                    {
                        self->onConnected(code);
                    }
                    else
                    {
                        ERROR_LOG(logger, "连接错误: 数据通道为空.");
                    }
                };
                pktExecutor->post("nac.connect", fn);
            }
            else
            {
                WARN_LOG(logger, "连接回调警告: 报文处理线程为空.");
            }
        });
        m_tcpClient->setDataCallback([wpSelf, wpPktExecutor, logger = m_logger](const std::vector<unsigned char>& data) {
            const auto pktExecutor = wpPktExecutor.lock();
            if (pktExecutor)
            {
                auto tp = std::chrono::steady_clock::now();
                auto fn = [wpSelf, tp, data, logger]() {
                    const auto& self = wpSelf.lock();
                    if (self)
                    {
                        self->sigUpdateRecvTime(tp);
                        self->onRecvData(data);
                    }
                    else
                    {
                        ERROR_LOG(logger, "数据接收错误: 数据通道为空.");
                    }
                };
                pktExecutor->post("nac.recv", fn);
            }
            else
            {
                WARN_LOG(logger, "数据接收警告: 报文处理线程为空.");
            }
        });
        const std::weak_ptr<nsocket::TcpClient> wpTcpClient = m_tcpClient;
        m_tcpExecutor->post("nac.loop", [wpTcpClient, address, port, filePEM, certFile, privateKeyFile, privateKeyFilePwd,
                                         logger = m_logger]() {
            const auto tcpClient = wpTcpClient.lock();
            if (tcpClient)
            {
                try
                {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                    std::shared_ptr<boost::asio::ssl::context> sslContext = nullptr;
                    if (!certFile.empty())
                    {
                        auto fmt = (filePEM ? boost::asio::ssl::context::file_format::pem : boost::asio::ssl::context::file_format::asn1);
                        if (privateKeyFile.empty()) /* SSL单向验证 */
                        {
                            sslContext = nsocket::TcpClient::getSsl1WayContext(fmt, certFile);
                        }
                        else /* SSL双向验证 */
                        {
                            sslContext = nsocket::TcpClient::getSsl2WayContext(fmt, certFile, privateKeyFile, privateKeyFilePwd);
                        }
                    }
                    tcpClient->run(address, port, sslContext);
#else
                    tcpClient->run(address, port);
#endif
                }
                catch (const std::exception& e)
                {
                    ERROR_LOG(logger, "运行异常: {}.", e.what());
                }
                catch (...)
                {
                    ERROR_LOG(logger, "运行异常: 未知原因.");
                }
            }
            else
            {
                ERROR_LOG(logger, "运行错误: 客户端为空.");
            }
        });
        return true;
    }
    catch (const std::exception& e)
    {
        ERROR_LOG(m_logger, "连接异常: {}.", e.what());
    }
    catch (...)
    {
        ERROR_LOG(m_logger, "连接异常: 未知原因.");
    }
    disconnectImpl();
    return false;
}

void DataChannel::disconnect()
{
    INFO_LOG(m_logger, "断开连接.");
    disconnectImpl();
}

bool DataChannel::isOpened() const
{
    return (m_tcpClient && m_tcpClient->isRunning());
}

boost::asio::ip::tcp::endpoint DataChannel::getLocalEndpoint() const
{
    if (m_tcpClient)
    {
        return m_tcpClient->getLocalEndpoint();
    }
    return boost::asio::ip::tcp::endpoint();
}

bool DataChannel::sendData(const std::vector<unsigned char>& data, const SendCallback& callback)
{
    auto dataLength = data.size();
    try
    {
        if (!isOpened())
        {
            ERROR_LOG(m_logger, "数据发送错误: 未连接.");
            if (callback)
            {
                callback(false, dataLength, 0);
            }
            return false;
        }
        const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
        const std::weak_ptr<threading::Executor> wpPktExecutor = m_pktExecutor;
        m_tcpClient->sendAsync(
            data, [wpSelf, wpPktExecutor, dataLength, callback, logger = m_logger](const boost::system::error_code& code, size_t length) {
                const auto pktExecutor = wpPktExecutor.lock();
                if (pktExecutor)
                {
                    auto tp = std::chrono::steady_clock::now();
                    auto fn = [wpSelf, dataLength, callback, tp, code, length, logger]() {
                        const auto self = wpSelf.lock();
                        if (code)
                        {
                            ERROR_LOG(logger, "数据发送错误: [{}] [{}].", code.value(), code.message());
                        }
                        else
                        {
                            if (self)
                            {
                                self->sigUpdateSendTime(tp);
                            }
                            else
                            {
                                ERROR_LOG(logger, "数据发送错误: 数据通道为空.");
                            }
                            if (length > 0 && length < dataLength)
                            {
                                ERROR_LOG(logger, "数据发送错误: 总长度 {}, 只发送 {}, 需要断开连接.", dataLength, length);
                                self->disconnectImpl();
                            }
                        }
                        if (callback)
                        {
                            callback(!code && (length == dataLength), dataLength, length);
                        }
                    };
                    pktExecutor->post("nac.sendcb", fn);
                }
                else
                {
                    WARN_LOG(logger, "数据发送警告: 报文处理线程为空.");
                    if (callback)
                    {
                        callback(!code && (length == dataLength), dataLength, length);
                    }
                }
            });
        return true;
    }
    catch (const std::exception& e)
    {
        ERROR_LOG(m_logger, "数据发送错误: {}.", e.what());
    }
    catch (...)
    {
        ERROR_LOG(m_logger, "数据发送错误: 未知原因.");
    }
    if (callback)
    {
        callback(false, dataLength, 0);
    }
    return false;
}

void DataChannel::disconnectImpl()
{
    try
    {
        if (m_tcpClient)
        {
            m_tcpClient->stop();
            m_tcpClient.reset();
        }
    }
    catch (const std::exception& e)
    {
        ERROR_LOG(m_logger, "断开连接错误: {}.", e.what());
    }
    catch (...)
    {
        ERROR_LOG(m_logger, "断开连接错误: 未知原因.");
    }
}

void DataChannel::onConnected(const boost::system::error_code& code)
{
    if (code)
    {
        ERROR_LOG(m_logger, "连接失败: [{}] [{}].", code.value(), code.message());
        disconnectImpl();
    }
    else
    {
        INFO_LOG(m_logger, "连接成功.");
    }
    sigConnectStatus(!code);
}

void DataChannel::onRecvData(const std::vector<unsigned char>& data)
{
    auto result = sigRecvData(data);
    if (!result.empty())
    {
        if (!result.front()) /* 如果数据处理失败, 则断开连接 */
        {
            ERROR_LOG(m_logger, "断开连接: 数据处理错误.");
            disconnectImpl();
        }
    }
}
} // namespace nac
