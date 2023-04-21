#include "protocol_adapter.h"

namespace nac
{
namespace tcli
{
void ProtocolAdapter::setDataChannel(const std::shared_ptr<DataChannel>& dataChannel)
{
    m_connections.clear();
    if (dataChannel)
    {
        const std::weak_ptr<ProtocolAdapter> wpSelf = shared_from_this();
        m_connections.emplace_back(dataChannel->sigConnectStatus.connect([wpSelf](bool isConnected) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onConnectStatusChanged(isConnected);
            }
        }));
        m_connections.emplace_back(dataChannel->sigRecvData.connect([wpSelf](const std::vector<unsigned char>& data) -> bool {
            const auto self = wpSelf.lock();
            if (self)
            {
                return self->onRecvData(data);
            }
            return false;
        }));
    }
    m_wpDataChannel = dataChannel;
}

bool ProtocolAdapter::sendPacket(const std::shared_ptr<Packet>& pkt, const DataChannel::SendCallback& callback)
{
    if (pkt)
    {
        const auto dataChannel = m_wpDataChannel.lock();
        if (dataChannel)
        {
            std::vector<unsigned char> buffer;
            if (onPacketToBuffer(pkt, buffer))
            {
                return dataChannel->sendData(buffer, callback);
            }
            if (callback)
            {
                callback(boost::system::errc::make_error_code(boost::system::errc::no_message_available), pkt->size(), 0);
            }
        }
        else
        {
            ERROR_LOG(m_logger, "数据包发送错误: 数据通道为空.");
            if (callback)
            {
                callback(boost::system::errc::make_error_code(boost::system::errc::not_connected), pkt->size(), 0);
            }
        }
    }
    else
    {
        ERROR_LOG(m_logger, "数据包发送错误: 数据包为空.");
        if (callback)
        {
            callback(boost::system::errc::make_error_code(boost::system::errc::no_message), 0, 0);
        }
    }
    return false;
}
} // namespace tcli
} // namespace nac
