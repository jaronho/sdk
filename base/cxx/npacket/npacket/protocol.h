#pragma once
#include <memory>
#include <string>

#include "protocol_raw.h"

namespace npacket
{
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
 * @brief 传输层协议(注意: 为了方便把第2次解析得出的协议类型归属到这里, 而并不仅包括标准的传输层协议)
 */
enum TransportProtocol
{
    TCP = 0x06,
    UDP = 0x11,
    ICMP = 0x01, /* 标准上归属于网络层协议 */
    ICMPv6 = 0x3a, /* 标准上归属于网络层协议 */
};

/**
 * @brief 协议头部(接口类)
 */
class ProtocolHeader
{
public:
    /**
     * @brief 获取(网络层/传输层)协议
     * @return 协议类型(NetworkProtocol/TransportProtocol)
     */
    virtual uint32_t getProtocol() const = 0;

    std::shared_ptr<ProtocolHeader> parent = nullptr; /* 父层头部 */
};

/**
 * @brief 以太网II协议头部
 */
class EthernetIIHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 14;
    }

    /**
     * @brief 获取协议类型
     * @return 物理层返回值恒为0
     */
    uint32_t getProtocol() const override
    {
        return 0;
    }

    /**
     * @brief 目标MAC地址字符串
     */
    std::string dstMacStr() const
    {
        char buf[18] = {0};
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", dstMac[0], dstMac[1], dstMac[2], dstMac[3], dstMac[4], dstMac[5]);
        return buf;
    }

    /**
     * @brief 源MAC地址字符串
     */
    std::string srcMacStr() const
    {
        char buf[18] = {0};
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", srcMac[0], srcMac[1], srcMac[2], srcMac[3], srcMac[4], srcMac[5]);
        return buf;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t dstMac[6]; /* 目标MAC地址 */
    uint8_t srcMac[6]; /* 源MAC地址 */
    uint32_t nextProtocol = 0; /* 下一层协议类型 */
};

/**
 * @brief IPv4协议头部
 */
class Ipv4Header final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 20;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层IPv4协议类型(NetworkProtocol)
     */
    uint32_t getProtocol() const override
    {
        return NetworkProtocol::IPv4;
    }

    /**
     * @brief 源IP地址字符串
     */
    std::string srcAddrStr() const
    {
        char buf[16] = {0};
        sprintf(buf, "%d.%d.%d.%d", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3]);
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    std::string dstAddrStr() const
    {
        char buf[16] = {0};
        sprintf(buf, "%d.%d.%d.%d", dstAddr[0], dstAddr[1], dstAddr[2], dstAddr[3]);
        return buf;
    }

    uint8_t version = 0; /* 版本 */
    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t tos = 0; /* 服务类型 */
    uint16_t totalLen = 0; /* 报文总长度 */
    uint16_t identification = 0; /* 标识 */
    uint8_t flagRsrvd = 0; /* 标志: 保留位 */
    uint8_t flagDont = 0; /* 标志: 没有分段 */
    uint8_t flagMore = 0; /* 标志: 更多分段 */
    uint16_t fragOffset = 0; /* 分段偏移数 */
    uint8_t ttl = 0; /* 报文生存时间 */
    uint8_t nextProtocol = 0; /* 下一层协议类型 */
    uint16_t checksum = 0; /* 头部校验和 */
    uint8_t srcAddr[4]; /* 源IP地址 */
    uint8_t dstAddr[4]; /* 目的IP地址 */
};

/**
 * @brief ARP协议头部
 */
class ArpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 28;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层ARP协议类型(NetworkProtocol)
     */
    uint32_t getProtocol() const override
    {
        return NetworkProtocol::ARP;
    }

    /**
     * @brief 源MAC地址字符串
     */
    std::string senderMacStr() const
    {
        char buf[18] = {0};
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);
        return buf;
    }

    /**
     * @brief 源IP地址字符串
     */
    std::string senderIpStr()
    {
        char buf[16] = {0};
        sprintf(buf, "%d.%d.%d.%d", senderIp[0], senderIp[1], senderIp[2], senderIp[3]);
        return buf;
    }

    /**
     * @brief 目标MAC地址字符串
     */
    std::string targetMacStr() const
    {
        char buf[18] = {0};
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", targetMac[0], targetMac[1], targetMac[2], targetMac[3], targetMac[4], targetMac[5]);
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    std::string targetIpStr() const
    {
        char buf[16] = {0};
        sprintf(buf, "%d.%d.%d.%d", targetIp[0], targetIp[1], targetIp[2], targetIp[3]);
        return buf;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint16_t hardwareType = 0; /* 硬件地址类型(表示物理网络类型, 即数据链路层使用的协议, 其中0x0001为以太网) */
    uint16_t protocolType = 0; /* 协议地址类型(网络层使用的协议) */
    uint8_t hardwareSize = 0; /* 硬件地址长度(源和目的物理地址的长度, 单位字节) */
    uint8_t protocolSize = 0; /* 协议地址长度(源和目的的协议地址的长度, 单位字节) */
    uint16_t opcode = 0; /* 操作(记录该报文的类型, 其中1表示ARP请求报文, 2表示ARP响应报文) */
    uint8_t senderMac[6]; /* 源MAC地址 */
    uint8_t senderIp[4]; /* 源IP地址(IPv4) */
    uint8_t targetMac[6]; /* 目的MAC地址 */
    uint8_t targetIp[4]; /* 目的IP地址(IPv4) */
};

/**
 * @brief IPv6协议头部
 */
class Ipv6Header final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 40;
    }

    /**
     * @brief 获取协议类型
     * @return 网络层IPv6协议类型(NetworkProtocol)
     */
    uint32_t getProtocol() const override
    {
        return NetworkProtocol::IPv6;
    }

    /**
     * @brief 源IP地址字符串
     */
    std::string srcAddrStr() const
    {
        char buf[40] = {0};
        sprintf(buf, "%x:%x:%x:%x:%x:%x:%x:%x", srcAddr[0], srcAddr[1], srcAddr[2], srcAddr[3], srcAddr[4], srcAddr[5], srcAddr[6],
                srcAddr[7]);
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    std::string dstAddrStr() const
    {
        char buf[40] = {0};
        sprintf(buf, "%x:%x:%x:%x:%x:%x:%x:%x", dstAddr[0], dstAddr[1], dstAddr[2], dstAddr[3], dstAddr[4], dstAddr[5], dstAddr[6],
                dstAddr[7]);
        return buf;
    }

    uint8_t version = 0; /* 版本 */
    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t trafficClass = 0; /* 通信类别 */
    uint32_t flowLabel = 0; /* 流标记 */
    uint16_t payloadLen = 0; /* 负载长度 */
    uint8_t nextHeader = 0; /* 下一个头的协议类型 */
    uint8_t hopLimit = 0; /* 跳跃限制 */
    uint16_t srcAddr[8]; /* 源IP地址 */
    uint16_t dstAddr[8]; /* 目的IP地址 */
    struct HopByHopHeader /* 逐跳选项头部(属于强制性的扩展头部), 长度为8的整数倍 */
    {
        uint8_t nextHeader = 0; /* 下一个头的协议类型 */
        uint8_t length = 0; /* 选项头长度(不包括前8个字节) */
        const uint8_t* options = nullptr; /* 选项(至少6个字节) */
        uint8_t optionLen = 6; /* 选项长度 */
    } hopByHopHeader;
};

/**
 * @brief TCP协议头部
 */
class TcpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 20;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层TCP协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const override
    {
        return TransportProtocol::TCP;
    }

    uint16_t srcPort = 0; /* 源端口 */
    uint16_t dstPort = 0; /* 目的端口 */
    uint32_t seq = 0; /* 序号 */
    uint32_t ack = 0; /* 确认序号 */
    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t flagRsrvd = 0; /* 标志: 保留位 */
    uint8_t flagNonce = 0; /* 标志: 保留位 */
    uint8_t flagCwr = 0; /* 标志: 保留位 */
    uint8_t flagEce = 0; /* 标志: 保留位 */
    uint8_t flagUrg = 0; /* 标志: 紧急 */
    uint8_t flagAck = 0; /* 标志: 确认 */
    uint8_t flagPsh = 0; /* 标志: 推送 */
    uint8_t flagRst = 0; /* 标志: 复位 */
    uint8_t flagSyn = 0; /* 标志: 同步 */
    uint8_t flagFin = 0; /* 标志: 终止 */
    uint16_t window = 0; /* 窗口大小 */
    uint16_t checksum = 0; /* 检验和 */
    uint16_t urgptr = 0; /* 紧急指针 */
};

/**
 * @brief UDP协议头部
 */
class UdpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 8;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层UDP协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const override
    {
        return TransportProtocol::UDP;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint16_t srcPort = 0; /* 源端口 */
    uint16_t dstPort = 0; /* 目的端口 */
    uint16_t totalLen = 0; /* 报文总长度 */
    uint16_t checksum = 0; /* 检验和 */
};

/**
 * @brief ICMP协议头部
 */
class IcmpHeader final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 3;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层ICMP协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const override
    {
        return TransportProtocol::ICMP;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t type = 0; /* 类型 */
    uint8_t code = 0; /* 代码 */
    uint16_t checksum = 0; /* 检验和 */
};

/**
 * @brief ICMPv6协议头部
 */
class Icmpv6Header final : public ProtocolHeader
{
public:
    /**
     * @brief 获取头部最小长度
     * @return 头部最小长度
     */
    static uint32_t getMinLen()
    {
        return 3;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层ICMPv6协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const override
    {
        return TransportProtocol::ICMPv6;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t type = 0; /* 类型 */
    uint8_t code = 0; /* 代码 */
    uint16_t checksum = 0; /* 检验和 */
};
} // namespace npacket
