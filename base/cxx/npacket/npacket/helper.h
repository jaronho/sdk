#pragma once
#include "protocol.h"

namespace npacket
{
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
    static uint16_t ntoh16(const uint8_t n[2]);

    /**
     * @brief 32位网络字节序转主机字节序
     * @param 32位网络字节序
     * @return 32位主机字节序
     */
    static uint32_t ntoh32(const uint8_t n[4]);

    /**
     * @brief 加载EthernetII头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<EthernetIIHeader> loadEthernetIIHeader(const RawEthernetIIHeader& r);

    /**
     * @brief 加载IPv4头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<Ipv4Header> loadIpv4Header(const RawIpv4Header& r);

    /**
     * @brief 加载ARP头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<ArpHeader> loadArpHeader(const RawArpHeader& r);

    /**
     * @brief 加载IPv6头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<Ipv6Header> loadIpv6Header(const RawIpv6Header& r);

    /**
     * @brief 加载TCP头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<TcpHeader> loadTcpHeader(const RawTcpHeader& r);

    /**
     * @brief 加载UDP头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<UdpHeader> loadUdpHeader(const RawUdpHeader& r);

    /**
     * @brief 加载ICMP头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<IcmpHeader> loadIcmpHeader(const RawIcmpHeader& r);

    /**
     * @brief 加载ICMPv6头部
     * @param r 原始头部
     * @return 头部
     */
    static std::shared_ptr<Icmpv6Header> loadIcmpv6Header(const RawIcmpv6Header& r);
};
} // namespace npacket
