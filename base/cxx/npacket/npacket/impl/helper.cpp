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
    p->header_len = EthernetIIHeader::getMinLen();
    char buf[3] = {0};
    for (int i = 0; i < 6; ++i)
    {
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%02x", r.dst_mac[i]);
        p->dst_mac.emplace_back(buf);
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%02x", r.src_mac[i]);
        p->src_mac.emplace_back(buf);
    }
    p->next_protocol = r.type[0] * 256 + r.type[1];
    return p;
}

std::shared_ptr<Ipv4Header> Helper::loadIpv4Header(const RawIpv4Header& r)
{
    auto p = std::make_shared<Ipv4Header>();
    p->version = r.ver_hl >> 4;
    p->header_len = (r.ver_hl & 0xF) * 4;
    p->tos = r.tos;
    p->total_len = ntoh16(r.total_len);
    p->identification = ntoh16(r.identification);
    auto frag_fo = ntoh16(r.frag_fo);
    p->flag_reserved = frag_fo & 0x8000;
    p->flag_dont = frag_fo & 0x4000;
    p->flag_more = frag_fo & 0x2000;
    p->frag_offset = frag_fo & 0x1FFF;
    p->ttl = r.ttl;
    p->next_protocol = r.protocol;
    p->checksum = ntoh16(r.checksum);
    char buf[16] = {0};
    sprintf_s(buf, sizeof(buf), "%d.%d.%d.%d", r.src_addr[0], r.src_addr[1], r.src_addr[2], r.src_addr[3]);
    p->src_addr = buf;
    memset(buf, 0, sizeof(buf));
    sprintf_s(buf, sizeof(buf), "%d.%d.%d.%d", r.dst_addr[0], r.dst_addr[1], r.dst_addr[2], r.dst_addr[3]);
    p->dst_addr = buf;
    return p;
}

std::shared_ptr<ArpHeader> Helper::loadArpHeader(const RawArpHeader& r)
{
    auto p = std::make_shared<ArpHeader>();
    p->header_len = ArpHeader::getMinLen();
    p->hardware_type = ntoh16(r.hardware_type);
    p->protocol_type = ntoh16(r.protocol_type);
    p->hardware_size = r.hardware_size;
    p->protocol_size = r.protocol_size;
    p->opcode = ntoh16(r.opcode);
    char buf[16] = {0};
    for (int i = 0; i < 6; ++i)
    {
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%02x", r.sender_mac[i]);
        p->sender_mac.emplace_back(buf);
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%02x", r.target_mac[i]);
        p->target_mac.emplace_back(buf);
    }
    memset(buf, 0, sizeof(buf));
    sprintf_s(buf, sizeof(buf), "%d.%d.%d.%d", r.sender_ip[0], r.sender_ip[1], r.sender_ip[2], r.sender_ip[3]);
    p->sender_ip = buf;
    memset(buf, 0, sizeof(buf));
    sprintf_s(buf, sizeof(buf), "%d.%d.%d.%d", r.target_ip[0], r.target_ip[1], r.target_ip[2], r.target_ip[3]);
    p->target_ip = buf;
    return p;
}

std::shared_ptr<Ipv6Header> Helper::loadIpv6Header(const RawIpv6Header& r)
{
    auto p = std::make_shared<Ipv6Header>();
    auto ver_flow = ntoh32(r.ver_flow);
    p->version = ver_flow >> 28;
    p->header_len = Ipv6Header::getMinLen();
    p->traffic_class = (ver_flow >> 20) & 0xFF;
    p->flow_label = ver_flow & 0xFFFFF;
    p->payload_len = ntoh16(r.payload_len);
    p->next_protocol = r.protocol;
    p->hop_limit = r.hop_limit;
    char buf[5] = {0};
    uint32_t num = 0;
    for (int i = 0; i < 16; i += 2)
    {
        num = r.src_addr[i] * 256 + r.src_addr[i + 1];
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%x", num);
        p->src_addr.emplace_back(buf);
        num = r.dst_addr[i] * 256 + r.dst_addr[i + 1];
        memset(buf, 0, sizeof(buf));
        sprintf_s(buf, sizeof(buf), "%x", num);
        p->dst_addr.emplace_back(buf);
    }
    return p;
}

std::shared_ptr<TcpHeader> Helper::loadTcpHeader(const RawTcpHeader& r)
{
    auto p = std::make_shared<TcpHeader>();
    p->src_port = ntoh16(r.src_port);
    p->dst_port = ntoh16(r.dst_port);
    p->seq = ntoh32(r.seq);
    p->ack = ntoh32(r.ack);
    auto frame = ntoh16(r.frame);
    p->header_len = (frame >> 12) * 4;
    p->flag_reserved = (frame >> 9) & 0x7;
    p->flag_nonce = (frame >> 8) & 0x1;
    p->flag_cwr = (frame >> 7) & 0x1;
    p->flag_ecn_echo = (frame >> 6) & 0x1;
    p->flag_urgent = (frame >> 5) & 0x1;
    p->flag_ack = (frame >> 4) & 0x1;
    p->flag_push = (frame >> 3) & 0x1;
    p->flag_reset = (frame >> 2) & 0x1;
    p->flag_syn = (frame >> 1) & 0x1;
    p->flag_fin = frame & 0x1;
    p->window = ntoh16(r.window);
    p->checksum = ntoh16(r.checksum);
    p->urgptr = ntoh16(r.urgptr);
    return p;
}

std::shared_ptr<UdpHeader> Helper::loadUdpHeader(const RawUdpHeader& r)
{
    auto p = std::make_shared<UdpHeader>();
    p->header_len = UdpHeader::getMinLen();
    p->src_port = ntoh16(r.src_port);
    p->dst_port = ntoh16(r.dst_port);
    p->total_len = ntoh16(r.total_len);
    p->checksum = ntoh16(r.checksum);
    return p;
}
} // namespace npacket
