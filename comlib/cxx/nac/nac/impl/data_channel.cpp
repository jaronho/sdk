#include "data_channel.h"

namespace nac
{
bool DataChannel::connect(const std::string& address, unsigned short port, const std::string& certFile, const std::string& privateKeyFile,
                          const std::string& privateKeyFilePwd)
{
    if (m_tcpClient)
    {
        WARN_LOG(m_logger, "连接失败: 重复连接.");
        return false;
    }
    try
    {
        INFO_LOG(m_logger, "连接服务器: {}:{}.", address, port);
        const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
        m_tcpClient = std::make_shared<nsocket::TcpClient>();
        m_tcpClient->setConnectCallback([wpSelf, logger = m_logger](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onConnected(code);
            }
            else
            {
                ERROR_LOG(logger, "连接错误: 数据通道为空.");
            }
        });
        m_tcpClient->setDataCallback([wpSelf, logger = m_logger](const std::vector<unsigned char>& data) {
            const auto& self = wpSelf.lock();
            if (self)
            {
                self->onRecvData(data);
            }
            else
            {
                ERROR_LOG(logger, "数据接收错误: 数据通道为空.");
            }
        });
        const std::weak_ptr<nsocket::TcpClient> wpTcpClient = m_tcpClient;
        m_tcpExecutor->post("nac_loop", [wpTcpClient, address, port, logger = m_logger]() {
            const auto tcpClient = wpTcpClient.lock();
            if (tcpClient)
            {
                try
                {
#if (1 == ENABLE_NSOCKET_OPENSSL)
                    auto sslContext = nsocket::TcpClient::getSsl2WayContext(certFile, privateKeyFile, privateKeyFilePwd);
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

bool DataChannel::isOpened()
{
    return (m_tcpClient && m_tcpClient->isRunning());
}

bool DataChannel::sendData(const std::vector<unsigned char>& data, const SendCallback& callback)
{
    if (!isOpened())
    {
        ERROR_LOG(m_logger, "数据发送错误: 未连接.");
        return false;
    }
    const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
    m_tcpClient->sendAsync(data, [wpSelf, callback, logger = m_logger](const boost::system::error_code& code, std::size_t length) {
        const auto self = wpSelf.lock();
        if (code)
        {
            ERROR_LOG(logger, "数据发送错误: [{}] [{}].", code.value(), code.message());
        }
        else
        {
            if (self)
            {
                self->sigUpdateSendTime();
            }
            else
            {
                ERROR_LOG(logger, "数据发送错误: 数据通道为空.");
            }
        }
        if (callback)
        {
            callback(!code, length);
        }
    });
    return true;
}

void DataChannel::disconnectImpl()
{
    if (m_tcpClient)
    {
        m_tcpClient->stop();
        m_tcpClient.reset();
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
    sigUpdateRecvTime();
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
