#include "analyzer.h"

#include "helper.h"

namespace npacket
{
void Analyzer::setLayerCallback(const LAYER_CALLBACK& ethernetLayerCb, const LAYER_CALLBACK& networkLayerCb,
                                const LAYER_CALLBACK& transportLayerCb)
{
    std::lock_guard<std::mutex> locker(m_mutexLayerCb);
    m_ethernetLayerCb = ethernetLayerCb;
    m_networkLayerCb = networkLayerCb;
    m_transportLayerCb = transportLayerCb;
}

bool Analyzer::addProtocolParser(const std::shared_ptr<ProtocolParser>& parser)
{
    if (parser)
    {
        std::lock_guard<std::mutex> locker(m_mutexParserList);
        for (auto item : m_applicationParserList)
        {
            if (item && item->getProtocol() == parser->getProtocol())
            {
                return false;
            }
        }
        m_applicationParserList.emplace_back(parser);
        return true;
    }
    return false;
}

void Analyzer::removeProtocolParser(uint32_t protocol)
{
    std::lock_guard<std::mutex> locker(m_mutexParserList);
    for (auto iter = m_applicationParserList.begin(); m_applicationParserList.end() != iter; ++iter)
    {
        if (*iter && (*iter)->getProtocol() == protocol)
        {
            m_applicationParserList.erase(iter);
            break;
        }
    }
}

int Analyzer::parse(const uint8_t* data, uint32_t dataLen)
{
    if (!data || 0 == dataLen)
    {
        return -1;
    }
    uint32_t remainLen = dataLen, offset = 0, headerLen = 0, networkProtocol = 0, transportProtocol = 0;
    LAYER_CALLBACK ehternetLayerCb = nullptr, networkLayerCb = nullptr, transportLayerCb = nullptr;
    {
        ehternetLayerCb = m_ethernetLayerCb;
        networkLayerCb = m_networkLayerCb;
        transportLayerCb = m_transportLayerCb;
    }
    /* 解析以太网层 */
    auto ethernetHeader = handleEthernetLayer(data + offset, remainLen, headerLen, networkProtocol);
    if (!ethernetHeader)
    {
        return 1;
    }
    remainLen -= headerLen;
    offset += headerLen;
    if (ehternetLayerCb)
    {
        if (!ehternetLayerCb(dataLen, ethernetHeader, data + offset, remainLen))
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
        if (networkLayerCb)
        {
            if (!networkLayerCb(dataLen, networkHeader, data + offset, remainLen))
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
            if (transportLayerCb)
            {
                if (!transportLayerCb(dataLen, transportHeader, data + offset, remainLen))
                {
                    return 0;
                }
            }
            /* 解析应用层 */
            if (remainLen > 0)
            {
                std::vector<std::shared_ptr<ProtocolParser>> applicationParserList;
                {
                    std::lock_guard<std::mutex> locker(m_mutexParserList);
                    applicationParserList = m_applicationParserList;
                }
                for (auto parser : applicationParserList)
                {
                    if (parser && parser->parse(dataLen, transportHeader, data + offset, remainLen))
                    {
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleEthernetLayer(const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                              uint32_t& networkProtocol)
{
    if (data && dataLen >= EthernetIIHeader::getMinLen())
    {
        auto* r = (npacket::RawEthernetIIHeader*)(data);
        if (r)
        {
            auto header = Helper::loadEthernetIIHeader(*r);
            headerLen = header->headerLen;
            networkProtocol = header->nextProtocol;
            return header;
        }
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleNetworkLayer(const uint32_t& networkProtocol, const uint8_t* data, uint32_t dataLen,
                                                             uint32_t& headerLen, uint32_t& transportProtocol)
{
    if (data)
    {
        switch ((NetworkProtocol)networkProtocol)
        {
        case NetworkProtocol::IPv4:
            if (dataLen >= Ipv4Header::getMinLen())
            {
                auto* r = (npacket::RawIpv4Header*)(data);
                if (r)
                {
                    auto header = Helper::loadIpv4Header(*r);
                    headerLen = header->headerLen;
                    transportProtocol = header->nextProtocol;
                    return header;
                }
            }
            break;
        case NetworkProtocol::ARP:
            if (dataLen >= ArpHeader::getMinLen())
            {
                auto* r = (npacket::RawArpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadArpHeader(*r);
                    headerLen = header->headerLen;
                    return header;
                }
            }
            break;
        case NetworkProtocol::IPv6:
            if (dataLen >= Ipv6Header::getMinLen())
            {
                auto* r = (npacket::RawIpv6Header*)(data);
                if (r)
                {
                    auto header = Helper::loadIpv6Header(*r);
                    headerLen = header->headerLen;
                    switch (header->nextHeader)
                    {
                    case 0: /* 逐跳选项头部 */
                        header->hopByHopHeader.options = data + headerLen + 2;
                        header->hopByHopHeader.optionLen = 6 + header->hopByHopHeader.length;
                        headerLen += 8 + header->hopByHopHeader.length;
                        transportProtocol = header->hopByHopHeader.nextHeader;
                        break;
                    default: // TODO: 其他扩展头部处理
                        transportProtocol = header->nextHeader;
                        break;
                    }
                    return header;
                }
            }
            break;
        }
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleTransportLayer(const uint32_t& transportProtocol, const uint8_t* data, uint32_t dataLen,
                                                               uint32_t& headerLen)
{
    if (data)
    {
        switch ((TransportProtocol)transportProtocol)
        {
        case TransportProtocol::TCP:
            if (dataLen >= TcpHeader::getMinLen())
            {
                auto* r = (npacket::RawTcpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadTcpHeader(*r);
                    headerLen = header->headerLen;
                    return header;
                }
            }
            break;
        case TransportProtocol::UDP:
            if (dataLen >= UdpHeader::getMinLen())
            {
                auto* r = (npacket::RawUdpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadUdpHeader(*r);
                    headerLen = header->headerLen;
                    return header;
                }
            }
            break;
        case TransportProtocol::ICMP:
            if (dataLen >= IcmpHeader::getMinLen())
            {
                auto* r = (npacket::RawIcmpHeader*)(data);
                if (r)
                {
                    auto header = Helper::loadIcmpHeader(*r);
                    headerLen = header->headerLen;
                    return header;
                }
            }
            break;
        case TransportProtocol::ICMPv6:
            if (dataLen >= Icmpv6Header::getMinLen())
            {
                auto* r = (npacket::RawIcmpv6Header*)(data);
                if (r)
                {
                    auto header = Helper::loadIcmpv6Header(*r);
                    headerLen = header->headerLen;
                    return header;
                }
            }
            break;
        }
    }
    return nullptr;
}
} // namespace npacket
