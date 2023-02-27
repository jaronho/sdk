#pragma once
#include "../protocol.h"

namespace npacket
{
/**
 * @brief ������
 */
class Helper final
{
public:
    /**
     * @brief 16λ�����ֽ���ת�����ֽ���
     * @param 16λ�����ֽ���
     * @return 16λ�����ֽ���
     */
    static uint16_t ntoh16(const uint8_t n[2]);

    /**
     * @brief 32λ�����ֽ���ת�����ֽ���
     * @param 32λ�����ֽ���
     * @return 32λ�����ֽ���
     */
    static uint32_t ntoh32(const uint8_t n[4]);

    /**
     * @brief ����EthernetIIͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<EthernetIIHeader> loadEthernetIIHeader(const RawEthernetIIHeader& r);

    /**
     * @brief ����IPv4ͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<Ipv4Header> loadIpv4Header(const RawIpv4Header& r);

    /**
     * @brief ����ARPͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<ArpHeader> loadArpHeader(const RawArpHeader& r);

    /**
     * @brief ����IPv6ͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<Ipv6Header> loadIpv6Header(const RawIpv6Header& r);

    /**
     * @brief ����TCPͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<TcpHeader> loadTcpHeader(const RawTcpHeader& r);

    /**
     * @brief ����UDPͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<UdpHeader> loadUdpHeader(const RawUdpHeader& r);

    /**
     * @brief ����ICMPͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<IcmpHeader> loadIcmpHeader(const RawIcmpHeader& r);

    /**
     * @brief ����ICMPv6ͷ��
     * @param r ԭʼͷ��
     * @return ͷ��
     */
    static std::shared_ptr<Icmpv6Header> loadIcmpv6Header(const RawIcmpv6Header& r);
};
} // namespace npacket
