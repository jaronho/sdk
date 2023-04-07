#pragma once
#include "npacket/protocol.h"

/**
 * @brief 打印以太网
 */
void printEthernet(const std::shared_ptr<npacket::EthernetIIHeader>& h);

/**
 * @brief 打印IPv4
 */
void printIPv4(const std::shared_ptr<npacket::Ipv4Header>& h);

/**
 * @brief 打印ARP
 */
void printARP(const std::shared_ptr<npacket::ArpHeader>& h);

/**
 * @brief 打印IPv6
 */
void printIPv6(const std::shared_ptr<npacket::Ipv6Header>& h);

/**
 * @brief 打印TCP
 */
void printTCP(const std::shared_ptr<npacket::TcpHeader>& h);

/**
 * @brief 打印UDP
 */
void printUDP(const std::shared_ptr<npacket::UdpHeader>& h);

/**
 * @brief 打印ICMP
 */
void printICMP(const std::shared_ptr<npacket::IcmpHeader>& h);

/**
 * @brief 打印ICMPv6
 */
void printICMPv6(const std::shared_ptr<npacket::Icmpv6Header>& h);

/**
 * @brief 打印传输层头部
 */
void printTransportHeader(const std::shared_ptr<npacket::ProtocolHeader>& h);
