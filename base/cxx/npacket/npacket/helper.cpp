#include "helper.h"

#include <string.h>

namespace npacket
{
uint16_t Helper::ntoh16(const uint8_t n[2])
{
    return ((uint16_t)(n[0]) << 8) | n[1];
}

uint32_t Helper::ntoh32(const uint8_t n[4])
{
    return ((uint32_t)(n[0]) << 24) | ((uint32_t)(n[1]) << 16) | ((uint32_t)(n[2]) << 8) | n[3];
}

std::unique_ptr<EthernetIIHeader> Helper::loadEthernetIIHeader(const RawEthernetIIHeader& r)
{
    auto header = std::make_unique<EthernetIIHeader>();
    header->headerLen = EthernetIIHeader::getMinLen();
    memcpy(header->dstMac, r.dstMac, sizeof(r.dstMac));
    memcpy(header->srcMac, r.srcMac, sizeof(r.srcMac));
    header->nextProtocol = ntoh16(r.type);
    return header;
}

std::unique_ptr<Ipv4Header> Helper::loadIpv4Header(const RawIpv4Header& r)
{
    auto header = std::make_unique<Ipv4Header>();
    header->version = r.ver_ihl >> 4;
    header->headerLen = (r.ver_ihl & 0xF) * 4;
    header->tos = r.tos;
    header->totalLen = ntoh16(r.totalLen);
    header->identification = ntoh16(r.identification);
    auto flags_fragoffset = ntoh16(r.flags_offset);
    header->flagRsrvd = flags_fragoffset >> 15;
    header->flagDont = (flags_fragoffset >> 14) & 0x1;
    header->flagMore = (flags_fragoffset >> 13) & 0x1;
    header->fragOffset = flags_fragoffset & 0x1FFF;
    header->ttl = r.ttl;
    header->nextProtocol = r.protocol;
    header->checksum = ntoh16(r.checksum);
    memcpy(header->srcAddr, r.srcAddr, sizeof(r.srcAddr));
    memcpy(header->dstAddr, r.dstAddr, sizeof(r.dstAddr));
    return header;
}

std::unique_ptr<ArpHeader> Helper::loadArpHeader(const RawArpHeader& r)
{
    auto header = std::make_unique<ArpHeader>();
    header->headerLen = ArpHeader::getMinLen();
    header->hardwareType = ntoh16(r.hardwareType);
    header->protocolType = ntoh16(r.protocolType);
    header->hardwareSize = r.hardwareSize;
    header->protocolSize = r.protocolSize;
    header->opcode = ntoh16(r.opcode);
    memcpy(header->senderMac, r.senderMac, sizeof(r.senderMac));
    memcpy(header->targetMac, r.targetMac, sizeof(r.targetMac));
    memcpy(header->senderIp, r.senderIp, sizeof(r.senderIp));
    memcpy(header->targetIp, r.targetIp, sizeof(r.targetIp));
    return header;
}

std::unique_ptr<Ipv6Header> Helper::loadIpv6Header(const RawIpv6Header& r)
{
    auto header = std::make_unique<Ipv6Header>();
    auto ver_class_label = ntoh32(r.ver_class_label);
    header->version = ver_class_label >> 28;
    header->headerLen = Ipv6Header::getMinLen();
    header->trafficClass = (ver_class_label >> 20) & 0xFF;
    header->flowLabel = ver_class_label & 0xFFFFF;
    header->payloadLen = ntoh16(r.payloadLen);
    header->nextHeader = r.nextHeader;
    header->hopLimit = r.hopLimit;
    for (int i = 0; i < 8; ++i)
    {
        header->srcAddr[i] = ntoh16(&r.srcAddr[i * 2]);
        header->dstAddr[i] = ntoh16(&r.dstAddr[i * 2]);
    }
    return header;
}

std::unique_ptr<TcpHeader> Helper::loadTcpHeader(const RawTcpHeader& r)
{
    auto header = std::make_unique<TcpHeader>();
    header->srcPort = ntoh16(r.srcPort);
    header->dstPort = ntoh16(r.dstPort);
    header->seq = ntoh32(r.seq);
    header->ack = ntoh32(r.ack);
    auto flags = ntoh16(r.flags);
    header->headerLen = (flags >> 12) * 4;
    header->flagRsrvd = (flags >> 9) & 0x7;
    header->flagNonce = (flags >> 8) & 0x1;
    header->flagCwr = (flags >> 7) & 0x1;
    header->flagEce = (flags >> 6) & 0x1;
    header->flagUrg = (flags >> 5) & 0x1;
    header->flagAck = (flags >> 4) & 0x1;
    header->flagPsh = (flags >> 3) & 0x1;
    header->flagRst = (flags >> 2) & 0x1;
    header->flagSyn = (flags >> 1) & 0x1;
    header->flagFin = flags & 0x1;
    header->window = ntoh16(r.window);
    header->checksum = ntoh16(r.checksum);
    header->urgptr = ntoh16(r.urgptr);
    return header;
}

std::unique_ptr<UdpHeader> Helper::loadUdpHeader(const RawUdpHeader& r)
{
    auto header = std::make_unique<UdpHeader>();
    header->headerLen = UdpHeader::getMinLen();
    header->srcPort = ntoh16(r.srcPort);
    header->dstPort = ntoh16(r.dstPort);
    header->totalLen = ntoh16(r.length);
    header->checksum = ntoh16(r.checksum);
    return header;
}

std::unique_ptr<IcmpHeader> Helper::loadIcmpHeader(const RawIcmpHeader& r)
{
    auto header = std::make_unique<IcmpHeader>();
    header->headerLen = IcmpHeader::getMinLen();
    header->type = r.type;
    header->code = r.code;
    header->checksum = ntoh16(r.checksum);
    return header;
}

std::unique_ptr<Icmpv6Header> Helper::loadIcmpv6Header(const RawIcmpv6Header& r)
{
    auto header = std::make_unique<Icmpv6Header>();
    header->headerLen = Icmpv6Header::getMinLen();
    header->type = r.type;
    header->code = r.code;
    header->checksum = ntoh16(r.checksum);
    return header;
}
} // namespace npacket
