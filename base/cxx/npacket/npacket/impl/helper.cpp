#include "helper.h"

namespace npacket
{
uint16_t Helper::ntoh16(const uint8_t n[2])
{
    uint16_t h = 0;
    h += (uint16_t)n[0] << 8;
    h += (uint16_t)n[1];
    return h;
}

uint32_t Helper::ntoh32(const uint8_t n[4])
{
    uint32_t h = 0;
    h += (uint32_t)n[0] << 24;
    h += (uint32_t)n[1] << 16;
    h += (uint32_t)n[2] << 8;
    h += (uint32_t)n[3];
    return h;
}

std::shared_ptr<EthernetIIHeader> Helper::loadEthernetIIHeader(const RawEthernetIIHeader& r)
{
    auto p = std::make_shared<EthernetIIHeader>();
    p->headerLen = EthernetIIHeader::getMinLen();
    for (int i = 0; i < 6; ++i)
    {
        p->dstMac[i] = r.dstMac[i];
        p->srcMac[i] = r.srcMac[i];
    }
    p->nextProtocol = r.type[0] * 256 + r.type[1];
    return p;
}

std::shared_ptr<Ipv4Header> Helper::loadIpv4Header(const RawIpv4Header& r)
{
    auto p = std::make_shared<Ipv4Header>();
    p->version = r.ver_ihl >> 4;
    p->headerLen = (r.ver_ihl & 0xF) * 4;
    p->tos = r.tos;
    p->totalLen = ntoh16(r.totalLen);
    p->identification = ntoh16(r.identification);
    auto flags_fragoffset = ntoh16(r.flags_offset);
    p->flagRsrvd = flags_fragoffset & 0x8000;
    p->flagDont = flags_fragoffset & 0x4000;
    p->flagMore = flags_fragoffset & 0x2000;
    p->fragOffset = flags_fragoffset & 0x1FFF;
    p->ttl = r.ttl;
    p->nextProtocol = r.protocol;
    p->checksum = ntoh16(r.checksum);
    for (int i = 0; i < 4; ++i)
    {
        p->srcAddr[i] = r.srcAddr[i];
        p->dstAddr[i] = r.dstAddr[i];
    }
    return p;
}

std::shared_ptr<ArpHeader> Helper::loadArpHeader(const RawArpHeader& r)
{
    auto p = std::make_shared<ArpHeader>();
    p->headerLen = ArpHeader::getMinLen();
    p->hardwareType = ntoh16(r.hardwareType);
    p->protocolType = ntoh16(r.protocolType);
    p->hardwareSize = r.hardwareSize;
    p->protocolSize = r.protocolSize;
    p->opcode = ntoh16(r.opcode);
    for (int i = 0; i < 6; ++i)
    {
        p->senderMac[i] = r.senderMac[i];
        p->targetMac[i] = r.targetMac[i];
    }
    for (int i = 0; i < 4; ++i)
    {
        p->senderIp[i] = r.senderIp[i];
        p->targetIp[i] = r.targetIp[i];
    }
    return p;
}

std::shared_ptr<Ipv6Header> Helper::loadIpv6Header(const RawIpv6Header& r)
{
    auto p = std::make_shared<Ipv6Header>();
    auto ver_class_label = ntoh32(r.ver_class_label);
    p->version = ver_class_label >> 28;
    p->headerLen = Ipv6Header::getMinLen();
    p->trafficClass = (ver_class_label >> 20) & 0xFF;
    p->flowLabel = ver_class_label & 0xFFFFF;
    p->payloadLen = ntoh16(r.payloadLen);
    p->nextProtocol = r.nextHeader;
    p->hopLimit = r.hopLimit;
    for (int i = 0; i < 16; i += 2)
    {
        p->srcAddr[i / 2] = r.srcAddr[i] * 256 + r.srcAddr[i + 1];
        p->dstAddr[i / 2] = r.dstAddr[i] * 256 + r.dstAddr[i + 1];
    }
    return p;
}

std::shared_ptr<TcpHeader> Helper::loadTcpHeader(const RawTcpHeader& r)
{
    auto p = std::make_shared<TcpHeader>();
    p->srcPort = ntoh16(r.srcPort);
    p->dstPort = ntoh16(r.dstPort);
    p->seq = ntoh32(r.seq);
    p->ack = ntoh32(r.ack);
    auto flags = ntoh16(r.flags);
    p->headerLen = (flags >> 12) * 4;
    p->flagRsrvd = (flags >> 9) & 0x7;
    p->flagNonce = (flags >> 8) & 0x1;
    p->flagCwr = (flags >> 7) & 0x1;
    p->flagEce = (flags >> 6) & 0x1;
    p->flagUrg = (flags >> 5) & 0x1;
    p->flagAck = (flags >> 4) & 0x1;
    p->flagPsh = (flags >> 3) & 0x1;
    p->flagRst = (flags >> 2) & 0x1;
    p->flagSyn = (flags >> 1) & 0x1;
    p->flagFin = flags & 0x1;
    p->window = ntoh16(r.window);
    p->checksum = ntoh16(r.checksum);
    p->urgptr = ntoh16(r.urgptr);
    return p;
}

std::shared_ptr<UdpHeader> Helper::loadUdpHeader(const RawUdpHeader& r)
{
    auto p = std::make_shared<UdpHeader>();
    p->headerLen = UdpHeader::getMinLen();
    p->srcPort = ntoh16(r.srcPort);
    p->dstPort = ntoh16(r.dstPort);
    p->totalLen = ntoh16(r.length);
    p->checksum = ntoh16(r.checksum);
    return p;
}
} // namespace npacket
