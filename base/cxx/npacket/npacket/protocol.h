#pragma once
#include <string.h>
#include <string>

#include "protocol_raw.h"

namespace npacket
{
/**
 * @brief 单字节转十进制字符串
 * @param val 单字节
 * @param p [输出]十进制字符串
 */
static inline void byteToDecStr(uint8_t val, char*& p)
{
    if (p)
    {
        if (val >= 100)
        {
            *p++ = '0' + val / 100;
            val %= 100;
        }
        if (val >= 10)
        {
            *p++ = '0' + val / 10;
        }
        *p++ = '0' + val % 10;
    }
}

/**
 * @brief 单字节转十六进制字符串
 * @param val 单字节
 * @param p [输出]十六进制字符串
 */
static inline void byteToHexStr(uint8_t val, char*& p)
{
    static const char hex[] = "0123456789abcdef";
    if (p)
    {
        *p++ = hex[val >> 4];
        *p++ = hex[val & 0xf];
    }
}

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
     * @brief 析构函数(必须要有)
     */
    virtual ~ProtocolHeader() = default;

    /**
     * @brief 获取(网络层/传输层)协议
     * @return 协议类型(NetworkProtocol/TransportProtocol)
     */
    virtual uint32_t getProtocol() const noexcept = 0;

    /**
     * @brief 重置对象状态(复用时需要)
     */
    virtual void reset()
    {
        parent = nullptr;
    }

    const ProtocolHeader* parent = nullptr; /* 父层头部 */
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
    uint32_t getProtocol() const noexcept override
    {
        return 0;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        headerLen = 0;
        memset(dstMac, 0, sizeof(dstMac));
        memset(srcMac, 0, sizeof(srcMac));
        nextProtocol = 0;
    }

    /**
     * @brief 目标MAC地址字符串
     */
    const char* dstMacStr() const
    {
        static thread_local char buf[18] = {0};
        char* p = buf;
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr(dstMac[i], p);
        }
        *p = '\0';
        return buf;
    }

    /**
     * @brief 源MAC地址字符串
     */
    const char* srcMacStr() const
    {
        static thread_local char buf[18] = {0};
        char* p = buf;
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr(srcMac[i], p);
        }
        *p = '\0';
        return buf;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t dstMac[6] = {0}; /* 目标MAC地址 */
    uint8_t srcMac[6] = {0}; /* 源MAC地址 */
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
    uint32_t getProtocol() const noexcept override
    {
        return NetworkProtocol::IPv4;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        version = 0;
        headerLen = 0;
        tos = 0;
        totalLen = 0;
        identification = 0;
        flagRsrvd = 0;
        flagDont = 0;
        flagMore = 0;
        fragOffset = 0;
        ttl = 0;
        nextProtocol = 0;
        checksum = 0;
        memset(srcAddr, 0, sizeof(srcAddr));
        memset(dstAddr, 0, sizeof(dstAddr));
    }

    /**
     * @brief 源IP地址字符串
     */
    const char* srcAddrStr() const
    {
        static thread_local char buf[16] = {0};
        char* p = buf;
        byteToDecStr(srcAddr[0], p);
        *p++ = '.';
        byteToDecStr(srcAddr[1], p);
        *p++ = '.';
        byteToDecStr(srcAddr[2], p);
        *p++ = '.';
        byteToDecStr(srcAddr[3], p);
        *p = '\0';
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    const char* dstAddrStr() const
    {
        static thread_local char buf[16] = {0};
        char* p = buf;
        byteToDecStr(dstAddr[0], p);
        *p++ = '.';
        byteToDecStr(dstAddr[1], p);
        *p++ = '.';
        byteToDecStr(dstAddr[2], p);
        *p++ = '.';
        byteToDecStr(dstAddr[3], p);
        *p = '\0';
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
    uint8_t srcAddr[4] = {0}; /* 源IP地址 */
    uint8_t dstAddr[4] = {0}; /* 目的IP地址 */
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
    uint32_t getProtocol() const noexcept override
    {
        return NetworkProtocol::ARP;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        headerLen = 0;
        hardwareType = 0;
        protocolType = 0;
        hardwareSize = 0;
        protocolSize = 0;
        opcode = 0;
        memset(senderMac, 0, sizeof(senderMac));
        memset(senderIp, 0, sizeof(senderIp));
        memset(targetMac, 0, sizeof(targetMac));
        memset(targetIp, 0, sizeof(targetIp));
    }

    /**
     * @brief 源MAC地址字符串
     */
    const char* senderMacStr() const
    {
        static thread_local char buf[18] = {0};
        char* p = buf;
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr(senderMac[i], p);
        }
        *p = '\0';
        return buf;
    }

    /**
     * @brief 源IP地址字符串
     */
    const char* senderIpStr() const
    {
        static thread_local char buf[16] = {0};
        char* p = buf;
        byteToDecStr(senderIp[0], p);
        *p++ = '.';
        byteToDecStr(senderIp[1], p);
        *p++ = '.';
        byteToDecStr(senderIp[2], p);
        *p++ = '.';
        byteToDecStr(senderIp[3], p);
        *p = '\0';
        return buf;
    }

    /**
     * @brief 目标MAC地址字符串
     */
    const char* targetMacStr() const
    {
        static thread_local char buf[18] = {0};
        char* p = buf;
        for (int i = 0; i < 6; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr(targetMac[i], p);
        }
        *p = '\0';
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    const char* targetIpStr() const
    {
        static thread_local char buf[16] = {0};
        char* p = buf;
        byteToDecStr(targetIp[0], p);
        *p++ = '.';
        byteToDecStr(targetIp[1], p);
        *p++ = '.';
        byteToDecStr(targetIp[2], p);
        *p++ = '.';
        byteToDecStr(targetIp[3], p);
        *p = '\0';
        return buf;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint16_t hardwareType = 0; /* 硬件地址类型(表示物理网络类型, 即数据链路层使用的协议, 其中0x0001为以太网) */
    uint16_t protocolType = 0; /* 协议地址类型(网络层使用的协议) */
    uint8_t hardwareSize = 0; /* 硬件地址长度(源和目的物理地址的长度, 单位字节) */
    uint8_t protocolSize = 0; /* 协议地址长度(源和目的的协议地址的长度, 单位字节) */
    uint16_t opcode = 0; /* 操作(记录该报文的类型, 其中1表示ARP请求报文, 2表示ARP响应报文) */
    uint8_t senderMac[6] = {0}; /* 源MAC地址 */
    uint8_t senderIp[4] = {0}; /* 源IP地址(IPv4) */
    uint8_t targetMac[6] = {0}; /* 目的MAC地址 */
    uint8_t targetIp[4] = {0}; /* 目的IP地址(IPv4) */
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
    uint32_t getProtocol() const noexcept override
    {
        return NetworkProtocol::IPv6;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        version = 0;
        headerLen = 0;
        trafficClass = 0;
        flowLabel = 0;
        payloadLen = 0;
        nextHeader = 0;
        hopLimit = 0;
        memset(srcAddr, 0, sizeof(srcAddr));
        memset(dstAddr, 0, sizeof(dstAddr));
        hopByHopHeader.nextHeader = 0;
        hopByHopHeader.length = 0;
        hopByHopHeader.options = nullptr;
        hopByHopHeader.optionLen = 6;
    }

    /**
     * @brief 源IP地址字符串
     */
    const char* srcAddrStr() const
    {
        static thread_local char buf[40] = {0};
        char* p = buf;
        for (int i = 0; i < 8; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr((srcAddr[i] >> 8) & 0xFF, p);
            byteToHexStr(srcAddr[i] & 0xFF, p);
        }
        *p = '\0';
        return buf;
    }

    /**
     * @brief 目的IP地址字符串
     */
    const char* dstAddrStr() const
    {
        static thread_local char buf[40] = {0};
        char* p = buf;
        for (int i = 0; i < 8; ++i)
        {
            if (i > 0)
            {
                *p++ = ':';
            }
            byteToHexStr((dstAddr[i] >> 8) & 0xFF, p);
            byteToHexStr(dstAddr[i] & 0xFF, p);
        }
        *p = '\0';
        return buf;
    }

    uint8_t version = 0; /* 版本 */
    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t trafficClass = 0; /* 通信类别 */
    uint32_t flowLabel = 0; /* 流标记 */
    uint16_t payloadLen = 0; /* 负载长度 */
    uint8_t nextHeader = 0; /* 下一个头的协议类型 */
    uint8_t hopLimit = 0; /* 跳跃限制 */
    uint16_t srcAddr[8] = {0}; /* 源IP地址 */
    uint16_t dstAddr[8] = {0}; /* 目的IP地址 */
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
    uint32_t getProtocol() const noexcept override
    {
        return TransportProtocol::TCP;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        srcPort = 0;
        dstPort = 0;
        seq = 0;
        ack = 0;
        headerLen = 0;
        flagRsrvd = 0;
        flagNonce = 0;
        flagCwr = 0;
        flagEce = 0;
        flagUrg = 0;
        flagAck = 0;
        flagPsh = 0;
        flagRst = 0;
        flagSyn = 0;
        flagFin = 0;
        window = 0;
        checksum = 0;
        urgptr = 0;
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
    uint32_t getProtocol() const noexcept override
    {
        return TransportProtocol::UDP;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        headerLen = 0;
        srcPort = 0;
        dstPort = 0;
        totalLen = 0;
        checksum = 0;
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
        return 8;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层ICMP协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const noexcept override
    {
        return TransportProtocol::ICMP;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        headerLen = 0;
        type = 0;
        code = 0;
        checksum = 0;
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
        return 8;
    }

    /**
     * @brief 获取协议类型
     * @return 传输层ICMPv6协议类型(TransportProtocol)
     */
    uint32_t getProtocol() const noexcept override
    {
        return TransportProtocol::ICMPv6;
    }

    /**
     * @brief 重置对象状态
     */
    void reset() override
    {
        ProtocolHeader::reset();
        headerLen = 0;
        type = 0;
        code = 0;
        checksum = 0;
    }

    uint8_t headerLen = 0; /* 头部长度 */
    uint8_t type = 0; /* 类型 */
    uint8_t code = 0; /* 代码 */
    uint16_t checksum = 0; /* 检验和 */
};

/**
 * @brief 分片缓存键值(唯一标识一组分片)
 */
struct FragmentKey
{
    /* IPv4分片标识: 源IP + 目的IP + Identification */
    struct Ipv4Key
    {
        uint8_t srcIp[4]; /* 源地址: 存储网络字节序数组 */
        uint8_t dstIp[4]; /* 目的地址: 存储网络字节序数组 */
        uint16_t identification;

        bool operator<(const Ipv4Key& other) const
        {
            int cmp = memcmp(srcIp, other.srcIp, sizeof(other.srcIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            cmp = memcmp(dstIp, other.dstIp, sizeof(other.dstIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            return (identification < other.identification);
        }
    };

    /* IPv6分片标识: 源IP + 目的IP + Fragment Header Identification */
    struct Ipv6Key
    {
        uint8_t srcIp[16]; /* 源地址: 存储网络字节序 */
        uint8_t dstIp[16]; /* 目的地址: 存储网络字节序 */
        uint32_t identification;

        bool operator<(const Ipv6Key& other) const
        {
            int cmp = memcmp(srcIp, other.srcIp, sizeof(other.srcIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            cmp = memcmp(dstIp, other.dstIp, sizeof(other.dstIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            return (identification < other.identification);
        }
    };

    uint8_t ipVersion = 0; /* IP版本: 4-IPv4, 6-IPv6 */
    Ipv4Key v4;
    Ipv6Key v6;

    /**
     * @brief 工厂函数：创建IPv4 Key
     * @param srcIp 源IP地址(4字节)
     * @param dstIp 目的IP地址(4字节)
     * @param identification 标识符
     * @return 分片键值
     */
    static FragmentKey createIpv4(const uint8_t srcIp[4], const uint8_t dstIp[4], uint16_t identification)
    {
        FragmentKey key;
        key.ipVersion = 4;
        memcpy(&key.v4.srcIp, srcIp, sizeof(key.v4.srcIp));
        memcpy(&key.v4.dstIp, dstIp, sizeof(key.v4.dstIp));
        key.v4.identification = identification;
        return key;
    }

    /**
     * @brief 工厂函数：创建IPv6 Key
     * @param srcIp 源IP地址(16字节)
     * @param dstIp 目的IP地址(16字节)
     * @param identification 标识符
     * @return 分片键值
     */
    static FragmentKey createIpv6(const uint8_t srcIp[16], const uint8_t dstIp[16], uint32_t identification)
    {
        FragmentKey key;
        key.ipVersion = 6;
        memcpy(&key.v6.srcIp, srcIp, sizeof(key.v6.srcIp));
        memcpy(&key.v6.dstIp, dstIp, sizeof(key.v6.dstIp));
        key.v6.identification = identification;
        return key;
    }

    bool operator==(const FragmentKey& other) const
    {
        if (ipVersion != other.ipVersion)
        {
            return false;
        }
        if (4 == ipVersion)
        {
            return (0 == memcmp(v4.srcIp, other.v4.srcIp, 4) && 0 == memcmp(v4.dstIp, other.v4.dstIp, 4)
                    && v4.identification == other.v4.identification);
        }
        return (0 == memcmp(v6.srcIp, other.v6.srcIp, 16) && 0 == memcmp(v6.dstIp, other.v6.dstIp, 16)
                && v6.identification == other.v6.identification);
    }

    bool operator<(const FragmentKey& other) const
    {
        if (ipVersion != other.ipVersion)
        {
            return ipVersion < other.ipVersion;
        }
        if (4 == ipVersion)
        {
            return (v4 < other.v4);
        }
        return (v6 < other.v6);
    }
};

/**
 * @brief TCP流标识键(四元组)
 */
struct TcpStreamKey
{
    /* IPv4地址 */
    struct Ipv4Key
    {
        uint8_t srcIp[4]; /* 源地址: 存储网络字节序数组 */
        uint8_t dstIp[4]; /* 目的地址: 存储网络字节序数组 */
        uint16_t srcPort; /* 源端口 */
        uint16_t dstPort; /* 目的端口 */

        bool operator<(const Ipv4Key& other) const
        {
            int cmp = memcmp(srcIp, other.srcIp, sizeof(srcIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            cmp = memcmp(dstIp, other.dstIp, sizeof(dstIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            if (srcPort != other.srcPort)
            {
                return srcPort < other.srcPort;
            }
            return dstPort < other.dstPort;
        }
    };

    /* IPv6地址 */
    struct Ipv6Key
    {
        uint8_t srcIp[16]; /* 源地址: 存储网络字节序 */
        uint8_t dstIp[16]; /* 目的地址: 存储网络字节序数组 */
        uint16_t srcPort; /* 源端口 */
        uint16_t dstPort; /* 目的端口 */

        bool operator<(const Ipv6Key& other) const
        {
            int cmp = memcmp(srcIp, other.srcIp, sizeof(srcIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            cmp = memcmp(dstIp, other.dstIp, sizeof(dstIp));
            if (0 != cmp)
            {
                return (cmp < 0);
            }
            if (srcPort != other.srcPort)
            {
                return srcPort < other.srcPort;
            }
            return dstPort < other.dstPort;
        }
    };

    uint8_t ipVersion = 0; /* IP版本: 4-IPv4, 6-IPv6 */
    Ipv4Key v4;
    Ipv6Key v6;

    /**
     * @brief 工厂函数：创建IPv4 Key
     * @param srcIp 源IP地址(4字节)
     * @param dstIp 目的IP地址(4字节)
     * @param srcPort 源端口
     * @param dstPort 目的端口
     * @return 流键值
     */
    static TcpStreamKey createIpv4(const uint8_t srcIp[4], const uint8_t dstIp[4], uint16_t srcPort, uint16_t dstPort)
    {
        TcpStreamKey key;
        key.ipVersion = 4;
        memcpy(key.v4.srcIp, srcIp, 4);
        memcpy(key.v4.dstIp, dstIp, 4);
        key.v4.srcPort = srcPort;
        key.v4.dstPort = dstPort;
        return key;
    }

    /**
     * @brief 工厂函数：创建IPv6 Key
     * @param srcIp 源IP地址(16字节)
     * @param dstIp 目的IP地址(16字节)
     * @param srcPort 源端口
     * @param dstPort 目的端口
     * @return 流键值
     */
    static TcpStreamKey createIpv6(const uint8_t srcIp[16], const uint8_t dstIp[16], uint16_t srcPort, uint16_t dstPort)
    {
        TcpStreamKey key;
        key.ipVersion = 6;
        memcpy(key.v6.srcIp, srcIp, 16);
        memcpy(key.v6.dstIp, dstIp, 16);
        key.v6.srcPort = srcPort;
        key.v6.dstPort = dstPort;
        return key;
    }

    bool operator==(const TcpStreamKey& other) const
    {
        if (ipVersion != other.ipVersion)
        {
            return false;
        }
        if (4 == ipVersion)
        {
            return (0 == memcmp(v4.srcIp, other.v4.srcIp, 4) && 0 == memcmp(v4.dstIp, other.v4.dstIp, 4) && v4.srcPort == other.v4.srcPort
                    && v4.dstPort == other.v4.dstPort);
        }
        return (0 == memcmp(v6.srcIp, other.v6.srcIp, 16) && 0 == memcmp(v6.dstIp, other.v6.dstIp, 16) && v6.srcPort == other.v6.srcPort
                && v6.dstPort == other.v6.dstPort);
    }

    bool operator<(const TcpStreamKey& other) const
    {
        if (ipVersion != other.ipVersion)
        {
            return (ipVersion < other.ipVersion);
        }
        if (4 == ipVersion)
        {
            return (v4 < other.v4);
        }
        return (v6 < other.v6);
    }
};
} // namespace npacket

namespace std
{
template<>
struct hash<npacket::FragmentKey>
{
    size_t operator()(const npacket::FragmentKey& k) const noexcept
    {
        size_t h = std::hash<uint8_t>{}(k.ipVersion);
        if (4 == k.ipVersion)
        {
            uint32_t srcIp, dstIp;
            memcpy(&srcIp, k.v4.srcIp, sizeof(srcIp));
            memcpy(&dstIp, k.v4.dstIp, sizeof(dstIp));
            h ^= std::hash<uint32_t>{}(srcIp) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(dstIp) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint16_t>{}(k.v4.identification) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        else
        {
            uint64_t src[2], dst[2];
            memcpy(&src, k.v6.srcIp, sizeof(src));
            memcpy(&dst, k.v6.dstIp, sizeof(dst));
            h ^= std::hash<uint64_t>{}(src[0]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(src[1]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(dst[0]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(dst[1]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(k.v6.identification) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

template<>
struct hash<npacket::TcpStreamKey>
{
    size_t operator()(const npacket::TcpStreamKey& k) const noexcept
    {
        size_t h = std::hash<uint8_t>{}(k.ipVersion);
        if (4 == k.ipVersion)
        {
            uint32_t srcIp, dstIp;
            memcpy(&srcIp, k.v4.srcIp, sizeof(srcIp));
            memcpy(&dstIp, k.v4.dstIp, sizeof(dstIp));
            h ^= std::hash<uint32_t>{}(srcIp) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(dstIp) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint16_t>{}(k.v4.srcPort) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint16_t>{}(k.v4.dstPort) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        else
        {
            uint64_t src[2], dst[2];
            memcpy(&src, k.v6.srcIp, sizeof(src));
            memcpy(&dst, k.v6.dstIp, sizeof(dst));
            h ^= std::hash<uint64_t>{}(src[0]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(src[1]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(dst[0]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint64_t>{}(dst[1]) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint16_t>{}(k.v6.srcPort) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint16_t>{}(k.v6.dstPort) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};
} // namespace std
