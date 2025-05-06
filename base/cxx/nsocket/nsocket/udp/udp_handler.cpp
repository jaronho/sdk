#include "udp_handler.h"

#include <chrono>

namespace nsocket
{
static std::atomic<uint64_t> s_timestamp{0}; /* 注意: std::atomic_uint64_t在某些平台下未定义 */
static std::atomic_int s_count{0};

UdpHandler::UdpHandler(const std::shared_ptr<SocketUdpBase>& socket, size_t bz) : m_socketUdpBase(socket)
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
    resizeBuffer(bz);
}

UdpHandler::~UdpHandler()
{
    close();
}

uint64_t UdpHandler::getId() const
{
    return m_id;
}

size_t UdpHandler::getBufferSize() const
{
    return m_recvBuf.size();
}

void UdpHandler::resizeBuffer(size_t bz)
{
    m_recvBuf.resize(bz > 128 ? bz : 128);
}

void UdpHandler::setOpenCallback(const UDP_OPEN_CALLBACK& onOpenCb)
{
    m_onOpenCallback = onOpenCb;
}

void UdpHandler::setDataCallback(const UDP_DATA_CALLBACK& onDataCb)
{
    m_onDataCallback = onDataCb;
}

void UdpHandler::setNonBlock(bool nonBlock)
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        socketUdpBase->setNonBlock(nonBlock);
    }
}

void UdpHandler::setSendBufferSize(int bufferSize)
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        socketUdpBase->setSendBufferSize(bufferSize);
    }
}

void UdpHandler::setRecvBufferSize(int bufferSize)
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        socketUdpBase->setRecvBufferSize(bufferSize);
    }
}

void UdpHandler::open(const boost::asio::ip::udp::endpoint& point, bool broadcast)
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        const std::weak_ptr<UdpHandler> wpSelf = shared_from_this();
        socketUdpBase->open(point, broadcast, [wpSelf](const boost::system::error_code& code) {
            const auto self = wpSelf.lock();
            if (self)
            {
                if (code) /* 打开失败 */
                {
                    self->closeImpl();
                    if (self->m_onOpenCallback)
                    {
                        self->m_onOpenCallback(code);
                    }
                }
                else /* 打开成功 */
                {
                    self->m_isOpened = true;
                    if (self->m_onOpenCallback)
                    {
                        self->m_onOpenCallback(code);
                    }
                    self->recv();
                }
            }
        });
    }
    else if (m_onOpenCallback)
    {
        m_onOpenCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
    }
}

void UdpHandler::send(const boost::asio::ip::udp::endpoint& point, const std::vector<unsigned char>& data,
                      const UDP_SEND_CALLBACK& onSendCb)
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase && m_isOpened)
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
                if (m_isOpened)
                {
                    socketUdpBase->send(point, boost::asio::buffer(data.data() + sentLength, data.size() - sentLength),
                                        [&code, &sentLength](const boost::system::error_code& ec, size_t length) {
                                            code = ec;
                                            sentLength += length;
                                        });
                    if (code) /* 发送失败 */
                    {
                        break;
                    }
                }
                else
                {
                    onSendCb(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
                    return;
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

void UdpHandler::UdpHandler::recv()
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        const std::weak_ptr<UdpHandler> wpSelf = shared_from_this();
        socketUdpBase->recv(boost::asio::buffer(m_recvBuf),
                            [wpSelf](const boost::asio::ip::udp::endpoint& point, const boost::system::error_code& code, size_t length) {
                                const auto self = wpSelf.lock();
                                if (self)
                                {
                                    std::vector<unsigned char> data;
                                    if (!code) /* 接收成功 */
                                    {
                                        const unsigned char* rawData = (const unsigned char*)self->m_recvBuf.data();
                                        if (rawData && length > 0)
                                        {
                                            data.insert(data.end(), rawData, rawData + length);
                                        }
                                    }
                                    if (self->m_onDataCallback)
                                    {
                                        self->m_onDataCallback(point, code, data);
                                    }
                                    if (self->m_isOpened)
                                    {
                                        self->recv(); /* 继续接收 */
                                    }
                                }
                            });
    }
    else
    {
        closeImpl();
        if (m_onOpenCallback)
        {
            m_onOpenCallback(boost::system::errc::make_error_code(boost::system::errc::not_a_socket));
        }
    }
}

void UdpHandler::closeImpl()
{
    m_isOpened = false;
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
        m_socketUdpBase.reset();
    }
    if (socketUdpBase)
    {
        socketUdpBase->close();
    }
}

void UdpHandler::close()
{
    bool openedFlag = m_isOpened;
    closeImpl();
    if (openedFlag && m_onOpenCallback)
    {
        m_onOpenCallback(boost::system::errc::make_error_code(boost::system::errc::interrupted));
    }
}

bool UdpHandler::isOpened() const
{
    return m_isOpened;
}

bool UdpHandler::isNonBlock()
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        return socketUdpBase->isNonBlock();
    }
    return false;
}

int UdpHandler::getSendBufferSize()
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        return socketUdpBase->getSendBufferSize();
    }
    return -1;
}

int UdpHandler::getRecvBufferSize()
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        return socketUdpBase->getRecvBufferSize();
    }
    return -1;
}

boost::asio::ip::udp::endpoint UdpHandler::getLocalEndpoint()
{
    std::shared_ptr<SocketUdpBase> socketUdpBase = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        socketUdpBase = m_socketUdpBase;
    }
    if (socketUdpBase)
    {
        return socketUdpBase->getLocalEndpoint();
    }
    return boost::asio::ip::udp::endpoint();
}
} // namespace nsocket
