#pragma once
#include <stdint.h>

namespace npacket
{
/**
 * 注意: 网络协议头部中的数据根据网络字节序以大端存储, 因此在使用时都需要转为主机字节序
 */

/**
 * @brief TCP/IP四层模型
 */
enum LayerType
{
    physical = 1, /* 物理层 */
    network = 2, /* 网络层 */
    transport = 3, /* 传输层 */
    application = 4 /* 应用层 */
};

/**
 * @brief 网络层协议类型
 */
enum NetworkProtocolType
{
    IPv4 = 0x0800,
    ARP = 0x0806,
    RARP = 0x8035,
    IPv6 = 0x86dd,
};

/**
 * @brief 传输层协议类型
 */
enum TransportProtocolType
{
    TCP = 0x06,
    UDP = 0x11,
};

/**
 * @brief 以太网II协议头部
 */
struct RawEthernetIIHeader
{
    uint8_t dst_mac[6]; /* 目标MAC地址 */
    uint8_t src_mac[6]; /* 源MAC地址 */
    uint8_t type[2]; /* 下一层协议类型 */
};

/**
 * @brief IPv4协议头部
 */
struct RawIpv4Header
{
    uint8_t ver_hl; /* 版本(4位)+头部长度(4位, 实际长度需要该值x4) */
    uint8_t tos; /* 服务类型 */
    uint8_t total_len[2]; /* 报文总长度 */
    uint8_t identification[2]; /* 标识 */
    uint8_t frag_fo[2]; /* 分段标志(3位)+分段偏移数(13位) */
    uint8_t ttl; /* 报文生存时间 */
    uint8_t protocol; /* 下一层协议类型 */
    uint8_t checksum[2]; /* 头部校验和 */
    uint8_t src_addr[4]; /* 源IP地址 */
    uint8_t dst_addr[4]; /* 目的IP地址 */
};

/**
 * @brief ARP协议头部
 */
struct RawArpHeader
{
    uint8_t hardware_type[2]; /* 硬件地址类型(表示物理网络类型, 即数据链路层使用的协议, 其中0x0001为以太网) */
    uint8_t protocol_type[2]; /* 协议地址类型(网络层使用的协议) */
    uint8_t hardware_size; /* 硬件地址长度(源和目的物理地址的长度, 单位字节) */
    uint8_t protocol_size; /* 协议地址长度(源和目的的协议地址的长度, 单位字节) */
    uint8_t opcode[2]; /* 操作(记录该报文的类型, 其中1表示ARP请求报文, 2表示ARP响应报文) */
    uint8_t sender_mac[6]; /* 源MAC地址 */
    uint8_t sender_ip[4]; /* 源IP地址 */
    uint8_t target_mac[6]; /* 目的MAC地址 */
    uint8_t target_ip[4]; /* 目的IP地址 */
};

/**
 * @brief IPv6协议头部
 */
struct RawIpv6Header
{
    uint8_t ver_flow[4]; /* 版本(4位)+通信类别(8位)+流标记(20位) */
    uint8_t payload_len[2]; /* 负载长度 */
    uint8_t protocol; /* 下一层协议类型 */
    uint8_t hop_limit; /* 跳跃限制 */
    uint8_t src_addr[16]; /* 源IP地址 */
    uint8_t dst_addr[16]; /* 目的IP地址 */
};

/**
 * @brief TCP协议头部
 */
struct RawTcpHeader
{
    uint8_t src_port[2]; /* 源端口 */
    uint8_t dst_port[2]; /* 目的端口 */
    uint8_t seq[4]; /* 序号 */
    uint8_t ack[4]; /* 确认序号 */
    /* 头部长度(4位, 实际长度需要该值x4)+reserved(3位)+nonce(1位)+cwr(1位)+ecn-echo(1位)+urgent(1位)+ack(1位)+push(1位)+reset(1位)+syn(1位)+fin(1位) */
    uint8_t frame[2];
    uint8_t window[2]; /* 窗口大小 */
    uint8_t checksum[2]; /* 检验和 */
    uint8_t urgptr[2]; /* 紧急指针 */
};

/**
 * @brief UDP协议头部
 */
struct RawUdpHeader
{
    uint8_t src_port[2]; /* 源端口 */
    uint8_t dst_port[2]; /* 目的端口 */
    uint8_t total_len[2]; /* 报文总长度 */
    uint8_t checksum[2]; /* 检验和 */
};
} // namespace npacket
