#include "protocol_adapter_custom.h"

#include "utility/bytearray/bytearray.h"

namespace nac
{
namespace tcli
{
static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024; /* 最大包体大小 */

ProtocolAdapterCustom::ProtocolAdapterCustom(int32_t version)
{
    m_payload = std::make_shared<nsocket::Payload>(PacketCustom::headSize());
    m_pkt = std::make_shared<PacketCustom>();
    m_pkt->version = version;
}

std::shared_ptr<Packet> ProtocolAdapterCustom::createPacket(int32_t bizCode, int64_t seqId, const std::string& data)
{
    auto pkt = std::make_shared<PacketCustom>();
    pkt->version = m_pkt->version;
    pkt->bizCode = bizCode;
    pkt->seqId = seqId;
    pkt->data = data;
    return pkt;
}

void ProtocolAdapterCustom::onConnectStatusChanged(const boost::system::error_code& code)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_payload->reset(); /* 网络变化, 清空缓存的负载数据 */
}

bool ProtocolAdapterCustom::onPacketToBuffer(const std::shared_ptr<Packet>& pkt, std::vector<unsigned char>& buffer)
{
    auto dataLength = PacketCustom::headSize() + pkt->data.size();
    if (dataLength >= MAX_BODY_SIZE) /* 限制数据包大小 */
    {
        ERROR_LOG(m_logger, "数据发送错误: 包体大小 {} 太长, 业务码 {}, 序列ID {}.", pkt->data.size(), pkt->bizCode, pkt->seqId);
        return false;
    }
    utility::ByteArray::write32(buffer, pkt->version, true); /* 版本号 */
    utility::ByteArray::write32(buffer, pkt->data.size(), true); /* 包体长度 */
    utility::ByteArray::write32(buffer, pkt->bizCode, true); /* 业务码 */
    utility::ByteArray::write64(buffer, pkt->seqId, true); /* 序列ID */
    buffer.insert(buffer.end(), pkt->data.data(), pkt->data.data() + pkt->data.size()); /* 包体数据 */
    return true;
}

bool ProtocolAdapterCustom::onRecvData(const std::vector<unsigned char>& data)
{
    bool ret = true;
    std::lock_guard<std::mutex> locker(m_mutex);
    m_payload->unpack(
        data,
        [&](const std::vector<unsigned char>& head) {
            size_t offset = 0;
            auto version = utility::ByteArray::read32(head.data() + offset, true); /* 版本号 */
            if (version != m_pkt->version)
            {
                ret = false;
                ERROR_LOG(m_logger, "数据解析错误: 包版本号 {} 与 {} 不匹配.", version, m_pkt->version);
                onPacketVersionMismatch(m_pkt->version, version);
                return -1;
            }
            offset += sizeof(int32_t);
            auto bodyLen = utility::ByteArray::read32(head.data() + offset, true); /* 包体长度 */
            if (bodyLen >= MAX_BODY_SIZE) /* 限制数据包大小 */
            {
                ret = false;
                ERROR_LOG(m_logger, "数据解析错误: 包体大小 {} 太长.", bodyLen);
                onPacketLengthAbnormal(MAX_BODY_SIZE, bodyLen);
                return -1;
            }
            offset += sizeof(int32_t);
            m_pkt->bizCode = utility::ByteArray::read32(head.data() + offset, true); /* 业务码 */
            offset += sizeof(int32_t);
            m_pkt->seqId = utility::ByteArray::read64(head.data() + offset, true); /* 序列ID */
            return bodyLen;
        },
        [&](const std::vector<unsigned char>& body) {
            m_pkt->data.clear();
            if (!body.empty())
            {
                m_pkt->data.insert(m_pkt->data.end(), body.begin(), body.end());
            }
            sigRecvPacket(m_pkt);
        });
    return ret;
}
} // namespace tcli
} // namespace nac
