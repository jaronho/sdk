#pragma once

namespace npacket
{
/**
 * @brief 协议类型定义
 */
enum class ProtocolType
{
    UNKNOWN = 0x00, /* 未知类型 */
    ETHERNET = 0x01, /* 以太网协议 */
    IPv4 = 0x02, /* IPv4协议 */
    IPv6 = 0x04, /* IPv6协议 */
    IP = IPv4 | IPv6, /* IP协议 */
    TCP = 0x08, /* TCP协议 */
    UDP = 0x10, /* UDP协议 */
    HTTP_REQUEST = 0x20, /* HTTP请求协议 */
    HTTP_RESPONSE = 0x40, /* HTTP响应协议 */
    HTTP = HTTP_REQUEST | HTTP_RESPONSE, /* HTTP协议 */
    ARP = 0x80, /* ARP协议 */
    VLAN = 0x100, /* VLAN协议 */
    ICMP = 0x200, /* ICMP协议(当前不支持) */
    PPPoE_SESSION = 0x400, /* PPPoE会话协议 */
    PPPoE_DISCOVERY = 0x800, /* PPPoE发现协议 */
    PPPoE = PPPoE_SESSION | PPPoE_DISCOVERY, /* PPPoE协议 */
    DNS = 0x1000, /* DNS协议 */
    MPLS = 0x2000, /* MPLS */
    GREv0 = 0x4000, /* GREv0协议 */
    GREv1 = 0x8000, /* GREv1协议 */
    GRE = GREv0 | GREv1, /* GRE协议 */
    PPP_PPTP = 0x10000, /* PPP for PPTP协议 */
    SSL = 0x20000, /* SSL/TLS协议 */
    SLL = 0x40000, /* SLL(Linux cooked capture)协议 */
    DHCP = 0x80000, /* DHCP/BOOTP协议 */
    NULL_LOOPBACK = 0x100000, /* Null/Loopback协议 */
    IGMP = 0xE00000, /* IGMP协议 */
    IGMPv1 = 0x200000, /* IGMPv1协议 */
    IGMPv2 = 0x400000, /* IGMPv2协议 */
    IGMPv3 = 0x800000, /* IGMPv3协议 */
    GENERIC_PAYLOAD = 0x1000000, /* 通用负载(没有指定协议) */
    VXLAN = 0x2000000, /* VXLAN协议 */
    SIP_REQUEST = 0x4000000, /* SIP请求协议 */
    SIP_RESPONSE = 0x8000000, /* SIP响应协议 */
    SIP = SIP_REQUEST | SIP_RESPONSE, /* SIP协议 */
    SDP = 0x10000000, /* SDP协议 */
    PACKET_TRAILER = 0x20000000, /* Packet trailer */
    RADIUS = 0x40000000, /* RADIUS协议 */
    GTPv1 = 0x80000000, /* GTPv1协议 */
    GTP = 0x80000000 /* GTP协议(当前和GTPv1一致) */
};

/**
 * @brief OSI模型层级定义
 */
enum class OsiModelLayer
{
    UNKNOWN = 0, /* 未知 */
    APPLICATION_LAYER = 7, /* 应用层 */
    PRESENTATION_LAYER = 6, /* 表示层 */
    SESSION_LAYER = 5, /* 会话层 */
    TRANSPORT_LAYER = 4, /* 传输层 */
    NETWORK_LAYER = 3, /* 网络层 */
    DATALINK_LAYER = 2, /* 数据链路层 */
    PHYSICAL_LAYER = 1 /* 物理层 */
};
} // namespace npacket
