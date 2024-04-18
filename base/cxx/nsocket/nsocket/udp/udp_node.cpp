#include "udp_node.h"

#include <chrono>

namespace nsocket
{
static std::atomic<uint64_t> s_timestamp{0}; /* 注意: std::atomic_uint64_t在某些平台下未定义 */
static std::atomic_int s_count{0};

UdpNode::UdpNode(size_t bz) : m_udpHandler(nullptr), m_bufferSize(bz), m_onOpenCallback(nullptr), m_onDataCallback(nullptr) {}

UdpNode::~UdpNode()
{
    stop();
};

void UdpNode::setOpenCallback(const UDP_OPEN_CALLBACK& onOpenCb)
{
    m_onOpenCallback = onOpenCb;
}

void UdpNode::setDataCallback(const UDP_DATA_CALLBACK& onDataCb)
{
    m_onDataCallback = onDataCb;
}

void UdpNode::run(const std::string& host, unsigned int port, bool broadcast)
{
    if (RunStatus::running == m_runStatus)
    {
        return;
    }
    boost::system::error_code code;
    auto endpoints = boost::asio::ip::udp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
    if (code || endpoints.empty())
    {
        if (m_onOpenCallback)
        {
            m_onOpenCallback(code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available));
        }
    }
    else
    {
        boost::asio::ip::udp::socket socket(m_ioContext);
        auto udpHandler = std::make_shared<UdpHandler>(std::make_shared<SocketUdp>(std::move(socket)), m_bufferSize);
        const std::weak_ptr<UdpNode> wpSelf = shared_from_this();
        udpHandler->setOpenCallback([wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->handleOpen(code);
            }
        });
        udpHandler->setDataCallback(m_onDataCallback);
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            m_udpHandler = udpHandler;
        }
        udpHandler->open(endpoints.begin()->endpoint(), broadcast);
        if (RunStatus::idle == m_runStatus)
        {
            m_runStatus = RunStatus::running;
            m_ioContext.run();
        }
        stop();
    }
}

boost::system::error_code UdpNode::send(const std::string& host, unsigned int port, const std::vector<unsigned char>& data,
                                        size_t& sentLength)
{
    auto code = boost::system::errc::make_error_code(boost::system::errc::not_connected);
    sentLength = 0;
    std::shared_ptr<UdpHandler> udpHandler = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        udpHandler = m_udpHandler;
    }
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped() && udpHandler)
    {
        auto endpoints = boost::asio::ip::udp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
        if (code || endpoints.empty())
        {
            return code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available);
        }
        auto point = endpoints.begin()->endpoint();
        udpHandler->send(point, data, [&code, &sentLength](const boost::system::error_code& ec, size_t length) {
            code = ec;
            sentLength += length;
        });
    }
    return code;
}

void UdpNode::sendAsync(const std::string& host, unsigned int port, const std::vector<unsigned char>& data,
                        const UDP_SEND_CALLBACK& onSendCb)
{
    std::shared_ptr<UdpHandler> udpHandler = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        udpHandler = m_udpHandler;
    }
    if (RunStatus::running == m_runStatus && !m_ioContext.stopped() && udpHandler)
    {
        boost::system::error_code code;
        auto endpoints = boost::asio::ip::udp::resolver(m_ioContext).resolve(host, std::to_string(port), code);
        if (code || endpoints.empty())
        {
            if (onSendCb)
            {
                onSendCb(code ? code : boost::system::errc::make_error_code(boost::system::errc::address_not_available), 0);
            }
        }
        else
        {
            auto point = endpoints.begin()->endpoint();
            const std::weak_ptr<UdpNode> wpSelf = shared_from_this();
            boost::asio::post(m_ioContext, [wpSelf, point, data, onSendCb]() {
                const auto self = wpSelf.lock();
                if (self)
                {
                    std::shared_ptr<UdpHandler> udpHandler = nullptr;
                    {
                        std::lock_guard<std::mutex> locker(self->m_mutex);
                        udpHandler = self->m_udpHandler;
                    }
                    if (RunStatus::running == self->m_runStatus && !self->m_ioContext.stopped() && udpHandler)
                    {
                        udpHandler->send(point, data, onSendCb);
                    }
                    else if (onSendCb)
                    {
                        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
                    }
                }
                else if (onSendCb)
                {
                    onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
                }
            });
        }
    }
    else if (onSendCb)
    {
        onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
    }
}

void UdpNode::stop()
{
    if (RunStatus::running == m_runStatus)
    {
        m_runStatus = RunStatus::stop;
        if (!m_ioContext.stopped())
        {
            m_ioContext.stop();
        }
        std::shared_ptr<UdpHandler> udpHandler = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            udpHandler = m_udpHandler;
            m_udpHandler.reset();
        }
        if (udpHandler)
        {
            udpHandler->close();
        }
    }
}

bool UdpNode::isRunning() const
{
    return (RunStatus::running == m_runStatus);
}

boost::asio::ip::udp::endpoint UdpNode::getLocalEndpoint()
{
    std::shared_ptr<UdpHandler> udpHandler = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        udpHandler = m_udpHandler;
    }
    if (udpHandler)
    {
        return udpHandler->getLocalEndpoint();
    }
    return boost::asio::ip::udp::endpoint();
}

void UdpNode::handleOpen(const boost::system::error_code& code)
{
    std::shared_ptr<UdpHandler> udpHandler = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        udpHandler = m_udpHandler;
    }
    if (!udpHandler || code) /* 处理器为空, 或打开失败 */
    {
        stop();
    }
    if (m_onOpenCallback)
    {
        m_onOpenCallback(code);
    }
}
} // namespace nsocket
