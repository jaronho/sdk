#include "data_channel.h"

namespace nac
{
namespace tcli
{
std::weak_ptr<threading::Executor> DataChannel::getPktExecutor()
{
    return m_pktExecutor;
}

bool DataChannel::connect(unsigned short localPort, const std::string& address, unsigned short port, bool sslOn, int sslWay, int certFmt,
                          const std::string& certFile, const std::string& pkFile, const std::string& pkPwd)
{
    try
    {
        if (m_tcpClient)
        {
            WARN_LOG(m_logger, "连接失败: 重复连接.");
            return false;
        }
        sslWay = sslWay < 1 ? 1 : (sslWay > 2 ? 2 : sslWay);
        certFmt = certFmt < 1 ? 1 : (certFmt > 2 ? 2 : certFmt);
        if (sslOn)
        {
            INFO_LOG(m_logger, "连接服务器: {}:{}, 通道加密: 是, SSL验证: {}.", address, port, 1 == sslWay ? "单向" : "双向");
        }
        else
        {
            INFO_LOG(m_logger, "连接服务器: {}:{}, 通道加密: 否.", address, port);
        }
        const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
        const std::weak_ptr<threading::Executor> wpPktExecutor = m_pktExecutor;
        m_tcpClient = std::make_shared<nsocket::TcpClient>(localPort);
        m_tcpClient->setConnectCallback([wpSelf, wpPktExecutor, logger = m_logger](const boost::system::error_code& code) {
            const auto pktExecutor = wpPktExecutor.lock();
            if (pktExecutor)
            {
                auto func = [wpSelf, code, logger]() {
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
                pktExecutor->post("nac.tcli.connect", func);
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
                auto ntp = std::chrono::steady_clock::now();
                auto func = [wpSelf, ntp, data, logger]() {
                    const auto& self = wpSelf.lock();
                    if (self)
                    {
                        self->sigUpdateRecvTime(ntp);
                        self->onRecvData(data);
                    }
                    else
                    {
                        ERROR_LOG(logger, "数据接收错误: 数据通道为空.");
                    }
                };
                pktExecutor->post("nac.tcli.recv", func);
            }
            else
            {
                WARN_LOG(logger, "数据接收警告: 报文处理线程为空.");
            }
        });
        const std::weak_ptr<nsocket::TcpClient> wpTcpClient = m_tcpClient;
        m_tcpExecutor->post("nac.tcli.loop",
                            [wpTcpClient, address, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, logger = m_logger]() {
                                const auto tcpClient = wpTcpClient.lock();
                                if (tcpClient)
                                {
                                    try
                                    {
                                        tcpClient->run(address, port, sslOn, sslWay, certFmt, certFile, pkFile, pkPwd);
                                        INFO_LOG(logger, "运行结束.");
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

bool DataChannel::sendData(const std::vector<unsigned char>& data, const nsocket::TCP_SEND_CALLBACK& callback)
{
    try
    {
        if (!isOpened())
        {
            ERROR_LOG(m_logger, "数据发送错误: 未连接.");
            if (callback)
            {
                callback(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            }
            return false;
        }
        const std::weak_ptr<DataChannel> wpSelf = shared_from_this();
        const std::weak_ptr<threading::Executor> wpPktExecutor = m_pktExecutor;
        m_tcpClient->sendAsync(data,
                               [wpSelf, wpPktExecutor, callback, logger = m_logger](const boost::system::error_code& code, size_t length) {
                                   const auto pktExecutor = wpPktExecutor.lock();
                                   if (pktExecutor)
                                   {
                                       auto ntp = std::chrono::steady_clock::now();
                                       auto func = [wpSelf, callback, ntp, code, length, logger]() {
                                           const auto self = wpSelf.lock();
                                           if (code)
                                           {
                                               ERROR_LOG(logger, "数据发送错误: [{}] [{}].", code.value(), code.message());
                                           }
                                           else
                                           {
                                               if (self)
                                               {
                                                   self->sigUpdateSendTime(ntp);
                                               }
                                               else
                                               {
                                                   ERROR_LOG(logger, "数据发送错误: 数据通道为空.");
                                               }
                                           }
                                           if (callback)
                                           {
                                               callback(code, length);
                                           }
                                       };
                                       pktExecutor->post("nac.tcli.sendcb", func);
                                   }
                                   else
                                   {
                                       WARN_LOG(logger, "数据发送警告: 报文处理线程为空.");
                                       if (callback)
                                       {
                                           callback(code, length);
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
        callback(boost::system::errc::make_error_code(boost::system::errc::io_error), 0);
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
        WARN_LOG(m_logger, "连接失败: [{}] [{}].", code.value(), code.message());
        disconnectImpl();
    }
    else
    {
        INFO_LOG(m_logger, "连接成功.");
    }
    sigConnectStatus(code);
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
} // namespace tcli
} // namespace nac
