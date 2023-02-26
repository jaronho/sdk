#pragma once
#include <stdint.h>

namespace npacket
{
/**
 * 注意: 网络协议头部中的数据根据网络字节序以大端存储, 因此在使用时都需要转为主机字节序
 */

/**
 * @brief 网络层协议
 */
enum NetworkProtocol
{
    IPv4 = 0x0800,
    ARP = 0x0806,
    IPv6 = 0x86dd,
};

/**
 * @brief 传输层协议
 */
enum TransportProtocol
{
    TCP = 0x06,
    UDP = 0x11,
};

/**
 * @brief 以太网II协议头部
 */
struct RawEthernetIIHeader
{
    uint8_t dstMac[6]; /* 目标MAC地址 */
    uint8_t srcMac[6]; /* 源MAC地址 */
    uint8_t type[2]; /* 下一层协议类型 */
};

/**
 * @brief IPv4协议头部(https://www.rfc-editor.org/rfc/rfc791.html)
 */
struct RawIpv4Header
{
    uint8_t ver_ihl; /* 版本(4位)+头部长度(4位, 实际长度需要该值x4) */
    uint8_t tos; /* 服务类型 */
    uint8_t totalLen[2]; /* 报文总长度 */
    uint8_t identification[2]; /* 标识 */
    uint8_t flags_offset[2]; /* 标志(3位)+分段偏移数(13位) */
    uint8_t ttl; /* 报文生存时间 */
    uint8_t protocol; /* 下一层协议类型 */
    uint8_t checksum[2]; /* 头部校验和 */
    uint8_t srcAddr[4]; /* 源IP地址 */
    uint8_t dstAddr[4]; /* 目的IP地址 */
};

/**
 * @brief ARP协议头部(https://www.rfc-editor.org/rfc/rfc826.html)
 */
struct RawArpHeader
{
    uint8_t hardwareType[2]; /* 硬件地址类型(表示物理网络类型, 即数据链路层使用的协议, 其中0x0001为以太网) */
    uint8_t protocolType[2]; /* 协议地址类型(网络层使用的协议) */
    uint8_t hardwareSize; /* 硬件地址长度(源和目的物理地址的长度, 单位字节) */
    uint8_t protocolSize; /* 协议地址长度(源和目的的协议地址的长度, 单位字节) */
    uint8_t opcode[2]; /* 操作(记录该报文的类型, 其中1表示ARP请求报文, 2表示ARP响应报文) */
    uint8_t senderMac[6]; /* 源MAC地址 */
    uint8_t senderIp[4]; /* 源IP地址(IPv4) */
    uint8_t targetMac[6]; /* 目的MAC地址 */
    uint8_t targetIp[4]; /* 目的IP地址(IPv4) */
};

/**
 * @brief IPv6协议头部(https://www.rfc-editor.org/rfc/rfc2460.html)
 */
struct RawIpv6Header
{
    uint8_t ver_class_label[4]; /* 版本(4位)+通信类别(8位)+流标记(20位) */
    uint8_t payloadLen[2]; /* 负载长度 */
    uint8_t nextHeader; /* 下一层协议类型 */
    uint8_t hopLimit; /* 跳跃限制 */
    uint8_t srcAddr[16]; /* 源IP地址 */
    uint8_t dstAddr[16]; /* 目的IP地址 */
};

/**
 * @brief TCP协议头部(https://www.rfc-editor.org/rfc/rfc793.html)
 */
struct RawTcpHeader
{
    uint8_t srcPort[2]; /* 源端口 */
    uint8_t dstPort[2]; /* 目的端口 */
    uint8_t seq[4]; /* 序号 */
    uint8_t ack[4]; /* 确认序号 */
    /* 头部长度(4位, 实际长度需要该值x4)+reserved(3位)+nonce(1位)+cwr(1位)+ecn-echo(1位)+urgent(1位)+ack(1位)+push(1位)+reset(1位)+syn(1位)+fin(1位) */
    uint8_t flags[2];
    uint8_t window[2]; /* 窗口大小 */
    uint8_t checksum[2]; /* 检验和 */
    uint8_t urgptr[2]; /* 紧急指针 */
};

/**
 * @brief UDP协议头部(https://www.rfc-editor.org/rfc/rfc768.html)
 */
struct RawUdpHeader
{
    uint8_t srcPort[2]; /* 源端口 */
    uint8_t dstPort[2]; /* 目的端口 */
    uint8_t length[2]; /* 报文总长度 */
    uint8_t checksum[2]; /* 检验和 */
};
} // namespace npacket
