#include "packet_analyzer.h"

#include "impl/helper.h"

namespace npacket
{
void PacketAnalyzer::setLayerCallback(const LAYER_CALLBACK& ethernetLayerCb, const LAYER_CALLBACK& networkLayerCb,
                                      const LAYER_CALLBACK& transportLayerCb)
{
    m_ethernetLayerCb = ethernetLayerCb;
    m_networkLayerCb = networkLayerCb;
    m_transportLayerCb = transportLayerCb;
}

int PacketAnalyzer::parse(const uint8_t* data, uint32_t dataLen)
{
    if (!data || 0 == dataLen)
    {
        return -1;
    }
    uint32_t remainLen = dataLen, offset = 0, headerLen = 0, networkProtocol = 0, transportProtocol = 0;
    /* 解析以太网层 */
    auto ethernetHeader = handleEthernetLayer(data + offset, remainLen, headerLen, networkProtocol);
    if (!ethernetHeader)
    {
        return 1;
    }
    remainLen -= headerLen;
    offset += headerLen;
    if (m_ethernetLayerCb)
    {
        if (!m_ethernetLayerCb(dataLen, ethernetHeader, data + offset, remainLen))
        {
            return 0;
        }
    }
    /* 解析网络层 */
    if (remainLen > 0)
    {
        auto networkHeader = handleNetworkLayer(networkProtocol, data + offset, remainLen, headerLen, transportProtocol);
        if (!networkHeader)
        {
            return 2;
        }
        networkHeader->parent = ethernetHeader;
        remainLen -= headerLen;
        offset += headerLen;
        if (m_networkLayerCb)
        {
            if (!m_networkLayerCb(dataLen, networkHeader, data + offset, remainLen))
            {
                return 0;
            }
        }
        /* 解析传输层 */
        if (remainLen > 0)
        {
            auto transportHeader = handleTransportLayer(transportProtocol, data + offset, remainLen, headerLen);
            if (!transportHeader)
            {
                return 3;
            }
            transportHeader->parent = networkHeader;
            remainLen -= headerLen;
            offset += headerLen;
            if (m_transportLayerCb)
            {
                m_transportLayerCb(dataLen, transportHeader, data + offset, remainLen);
            }
        }
    }
    return 0;
}

std::shared_ptr<ProtocolHeader> PacketAnalyzer::handleEthernetLayer(const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                                    uint32_t& networkProtocol)
{
    if (data && dataLen >= EthernetIIHeader::getMinLen())
    {
        npacket::RawEthernetIIHeader* r = (npacket::RawEthernetIIHeader*)(data);
        if (r)
        {
            auto header = Helper::loadEthernetIIHeader(*r);
            headerLen = header->header_len;
            networkProtocol = header->next_protocol;
            return header;
        }
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> PacketAnalyzer::handleNetworkLayer(const uint32_t& networkProtocol, const uint8_t* data, uint32_t dataLen,
                                                                   uint32_t& headerLen, uint32_t& transportProtocol)
{
    if (data)
    {
        switch ((NetworkProtocol)networkProtocol)
        {
        case NetworkProtocol::IPv4:
            if (dataLen >= Ipv4Header::getMinLen())
            {
                npacket::RawIpv4Header* r = (npacket::RawIpv4Header*)(data);
                if (r)
                {
                    auto header = Helper::loadIpv4Header(*r);
                    headerLen = header->header_len;
                    transportProtocol = header->next_protocol;
                    return header;
                }
            }
            break;
        case NetworkProtocol::ARP:
            if (dataLen >= ArpHeader::getMinLen())
            {
                npacket::RawArpHeader* r = (npacket::RawArpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadArpHeader(*r);
                    headerLen = header->header_len;
                    return header;
                }
            }
            break;
        case NetworkProtocol::IPv6:
            if (dataLen >= Ipv6Header::getMinLen())
            {
                npacket::RawIpv6Header* r = (npacket::RawIpv6Header*)(data);
                if (r)
                {
                    auto header = Helper::loadIpv6Header(*r);
                    headerLen = header->header_len;
                    transportProtocol = header->next_protocol;
                    // TODO: 扩展包头处理
                    return header;
                }
            }
            break;
        }
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> PacketAnalyzer::handleTransportLayer(const uint32_t& transportProtocol, const uint8_t* data,
                                                                     uint32_t dataLen, uint32_t& headerLen)
{
    if (data)
    {
        switch ((TransportProtocol)transportProtocol)
        {
        case TransportProtocol::TCP:
            if (dataLen >= TcpHeader::getMinLen())
            {
                npacket::RawTcpHeader* r = (npacket::RawTcpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadTcpHeader(*r);
                    headerLen = header->header_len;
                    return header;
                }
            }
            break;
        case TransportProtocol::UDP:
            if (dataLen >= UdpHeader::getMinLen())
            {
                npacket::RawUdpHeader* r = (npacket::RawUdpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadUdpHeader(*r);
                    headerLen = header->header_len;
                    return header;
                }
            }
            break;
        }
    }
    return nullptr;
}
} // namespace npacket
