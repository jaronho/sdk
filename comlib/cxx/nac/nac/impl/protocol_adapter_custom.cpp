#include "protocol_adapter_custom.h"

#include "utility/bytearray/bytearray.h"

namespace nac
{
static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024; /* 最大包体大小 */

/**
 * @brief 包头
 */
struct PacketHead
{
    int32_t bodyLen = 0; /* 包体长度(4个字节) */
    int32_t bizCode = 0; /* 业务码(4个字节) */
    int64_t seqId = 0; /* 序列ID(8个字节) */
};

ProtocolAdapterCustom::ProtocolAdapterCustom()
{
    m_payload = std::make_shared<nsocket::Payload>(sizeof(PacketHead));
}

bool ProtocolAdapterCustom::sendPacket(const std::shared_ptr<Packet>& pkt, const DataChannel::SendCallback& callback)
{
    auto dataLength = m_payload->getHeadLen() + pkt->data.size();
    if (dataLength >= MAX_BODY_SIZE) /* 限制数据包大小 */
    {
        ERROR_LOG(m_logger, "数据发送错误: 包体大小 {} 太长, 业务码 {}, 序列ID {}.", pkt->data.size(), pkt->bizCode, pkt->seqId);
        if (callback)
        {
            callback(false, dataLength, 0);
        }
        return false;
    }
    const auto dataChannel = m_wpDataChannel.lock();
    if (dataChannel)
    {
        std::vector<unsigned char> buffer;
        utility::ByteArray::write32(buffer, pkt->data.size(), true); /* 包体长度 */
        utility::ByteArray::write32(buffer, pkt->bizCode, true); /* 业务码 */
        utility::ByteArray::write64(buffer, pkt->seqId, true); /* 序列ID */
        buffer.insert(buffer.end(), pkt->data.data(), pkt->data.data() + pkt->data.size()); /* 包体数据 */
        return dataChannel->sendData(buffer, callback);
    }
    else
    {
        ERROR_LOG(m_logger, "数据包发送错误: 数据通道为空.");
        if (callback)
        {
            callback(false, dataLength, 0);
        }
    }
    return false;
}

void ProtocolAdapterCustom::onConnectStatusChanged(bool isConnected)
{
    std::lock_guard<std::mutex> locker(m_mutexPayload);
    m_payload->reset(); /* 网络变化, 清空缓存的负载数据 */
}

bool ProtocolAdapterCustom::onRecvData(const std::vector<unsigned char>& data)
{
    static PacketHead s_pktHead;
    bool ret = true;
    std::lock_guard<std::mutex> locker(m_mutexPayload);
    m_payload->unpack(
        data,
        [&](const std::vector<unsigned char>& head) {
            int offset = 0;
            s_pktHead.bodyLen = utility::ByteArray::read32(head.data() + offset, true); /* 包体长度 */
            if (m_payload->getHeadLen() + (size_t)s_pktHead.bodyLen >= MAX_BODY_SIZE) /* 限制数据包大小 */
            {
                ret = false;
                ERROR_LOG(m_logger, "数据解析错误: 包体大小 {} 太长.", s_pktHead.bodyLen);
                return -1;
            }
            offset += sizeof(s_pktHead.bodyLen);
            s_pktHead.bizCode = utility::ByteArray::read32(head.data() + offset, true); /* 业务码 */
            offset += sizeof(s_pktHead.bizCode);
            s_pktHead.seqId = utility::ByteArray::read64(head.data() + offset, true); /* 序列ID */
            return s_pktHead.bodyLen;
        },
        [&](const std::vector<unsigned char>& body) {
            auto pkt = std::make_shared<Packet>();
            pkt->bizCode = s_pktHead.bizCode;
            pkt->seqId = s_pktHead.seqId;
            if (!body.empty())
            {
                pkt->data.insert(pkt->data.end(), body.begin(), body.end());
            }
            sigRecvPacket(pkt);
        });
    return ret;
}
} // namespace nac
