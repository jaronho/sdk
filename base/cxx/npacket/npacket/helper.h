#pragma once
#include "protocol.h"

namespace npacket
{
/**
 * @brief IPv6扩展头类型定义
 */
#define IPV6_EXT_HOP_BY_HOP 0 /* 逐跳选项头 */
#define IPV6_EXT_ROUTING 43 /* 路由头 */
#define IPV6_EXT_FRAGMENT 44 /* 分片头 */
#define IPV6_EXT_ESP 50 /* ESP头 */
#define IPV6_EXT_AUTH 51 /* 认证头 */
#define IPV6_EXT_DEST_OPTIONS 60 /* 目的选项头 */
#define IPV6_EXT_NO_NEXT 59 /* 无下一个头 */

/**
 * @brief 辅助类
 */
class Helper final
{
public:
    /**
     * @brief 16位网络字节序转主机字节序
     * @param 16位网络字节序
     * @return 16位主机字节序
     */
    inline static uint16_t ntoh16(const uint8_t n[2])
    {
        return ((uint16_t)(n[0]) << 8) | n[1];
    }

    /**
     * @brief 32位网络字节序转主机字节序
     * @param 32位网络字节序
     * @return 32位主机字节序
     */
    inline static uint32_t ntoh32(const uint8_t n[4])
    {
        return ((uint32_t)(n[0]) << 24) | ((uint32_t)(n[1]) << 16) | ((uint32_t)(n[2]) << 8) | n[3];
    }

    /**
     * @brief 加载EthernetII头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadEthernetIIHeader(const RawEthernetIIHeader& r, EthernetIIHeader& header);

    /**
     * @brief 加载IPv4头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadIpv4Header(const RawIpv4Header& r, Ipv4Header& header);

    /**
     * @brief 加载ARP头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadArpHeader(const RawArpHeader& r, ArpHeader& header);

    /**
     * @brief 加载IPv6头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadIpv6Header(const RawIpv6Header& r, Ipv6Header& header);

    /**
     * @brief 加载TCP头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadTcpHeader(const RawTcpHeader& r, TcpHeader& header);

    /**
     * @brief 加载UDP头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadUdpHeader(const RawUdpHeader& r, UdpHeader& header);

    /**
     * @brief 加载ICMP头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadIcmpHeader(const RawIcmpHeader& r, IcmpHeader& header);

    /**
     * @brief 加载ICMPv6头部
     * @param r 原始头部
     * @param header [输出]头部对象
     */
    static void loadIcmpv6Header(const RawIcmpv6Header& r, Icmpv6Header& header);
};
} // namespace npacket
