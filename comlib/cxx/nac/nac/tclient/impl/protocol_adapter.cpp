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
        m_connections.emplace_back(dataChannel->sigConnectStatus.connect([wpSelf](const boost::system::error_code& code) -> void {
            const auto self = wpSelf.lock();
            if (self)
            {
                self->onConnectStatusChanged(code);
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

void ProtocolAdapter::setPacketVersionMismatchCallback(const PACKET_VERSION_MISMATCH_CALLBACK& callback)
{
    m_packetVersionMismatchCb = callback;
}

void ProtocolAdapter::setPacketLengthAbnormalCallback(const PACKET_LENGTH_ABNORMAL_CALLBACK& callback)
{
    m_packetLengthAbnormalCb = callback;
}

bool ProtocolAdapter::sendPacket(const std::shared_ptr<Packet>& pkt, const nsocket::TCP_SEND_CALLBACK& callback)
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
            ERROR_LOG(m_logger, "数据包发送错误: 数据包不正确.");
            if (callback)
            {
                callback(boost::system::errc::make_error_code(boost::system::errc::no_message_available), 0);
            }
        }
        else
        {
            ERROR_LOG(m_logger, "数据包发送错误: 数据通道为空.");
            if (callback)
            {
                callback(boost::system::errc::make_error_code(boost::system::errc::not_connected), 0);
            }
        }
    }
    else
    {
        ERROR_LOG(m_logger, "数据包发送错误: 数据包为空.");
        if (callback)
        {
            callback(boost::system::errc::make_error_code(boost::system::errc::no_message), 0);
        }
    }
    return false;
}

void ProtocolAdapter::onPacketVersionMismatch(int32_t localVersion, int32_t pktVersion)
{
    if (m_packetVersionMismatchCb)
    {
        m_packetVersionMismatchCb(localVersion, pktVersion);
    }
}

void ProtocolAdapter::onPacketLengthAbnormal(int32_t maxLength, int32_t pktLength)
{
    if (m_packetLengthAbnormalCb)
    {
        m_packetLengthAbnormalCb(maxLength, pktLength);
    }
}
} // namespace tcli
} // namespace nac
