#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "protocol.h"
#include "protocol_parser.h"

namespace npacket
{
/**
 * @brief 层数据回调
 * @param num 数据序号
 * @param ntp 数据包接收时间点
 * @param totalLen 数据包总长度
 * @param header 层头部
 * @param payload 层负载
 * @param payloadLen 层负载长度
 * @return true-继续处理下一层, false-停止后续处理
 */
using LAYER_CALLBACK = std::function<bool(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                          const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)>;

/**
 * @brief 数据源
 */
enum DataSource
{
    NETWORK_ETH = 0, /* 网络包(以太网帧) */
    NETWORK_IPv4, /* 网络包(IPv4包) */
    NETWORK_IPv6, /* 网络包(IPv6包) */
    SERIAL, /* 串口数据包 */
};

/**
 * @brief 回调配置
 */
struct CallbackConfig
{
    LAYER_CALLBACK ethernetLayerCb = nullptr; /* 以太网层数据回调 */
    LAYER_CALLBACK networkLayerCb = nullptr; /* 网络层数据回调 */
    LAYER_CALLBACK transportLayerCb = nullptr; /* 传输层数据回调 */
};

/**
 * @brief IP分片重组配置
 */
struct IpReassemblyConfig
{
    bool enable = true; /* 是否启用IP分片重组功能 */
    size_t fragTimeout = 30000; /* 分片重组超时时间(毫秒) */
    size_t fragClearInterval = 6000; /* 分片缓存清理间隔(毫秒) */
    size_t maxFragSize = 8192; /* 单个分片最大负载大小(防止大分片攻击) */
    size_t maxFragmentCount = 32; /* 每组分片最大数量(防止分片攻击) */
    size_t maxReassembleSize = 65535; /* 最大重组后数据包大小(防止分片攻击) */
    size_t maxCacheCount = 500; /* 最大分片缓存数量(防止分片攻击) */
    size_t maxRecursionDepth = 3; /* 最大递归深度(防止分片嵌套攻击) */
};

/**
 * @brief TCP流重组配置
 */
struct TcpReassemblyConfig
{
    bool enable = true; /* 是否启用IP分片重组功能 */
    size_t streamTimeout = 30000; /* 流超时时间(毫秒) */
    size_t streamClearInterval = 6000; /* 流缓存清理间隔(毫秒) */
    size_t maxStreamSize = 1048576; /* 单个流最大缓存大小(1MB) */
    size_t maxStreamCount = 1000; /* 最大流数量 */
    size_t maxSegmentsPerStream = 32; /* 单流最大乱序段数(防止DoS攻击) */
    size_t finWaitTimeout = 5000; /* 收到FIN后等待乱序数据重组的超时时间(毫秒) */
    size_t gapSizeThreshold = 4096; /* 尽力交付(Gap Tolerance)阈值(字节), 0-无限等待丢包数据, >0-当>=该值(跳过丢包), 当<该值(流重置) */
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

namespace npacket
{
/**
 * @brief 分析器
 */
class Analyzer
{
public:
    /**
     * @brief 构造函数
     * @param cbCfg 回调配置
     * @param ipReassemblyCfg IP分片重组配置
     * @param tcpReassemblyCfg TCP流重组配置
     */
    Analyzer(const CallbackConfig& cbCfg = CallbackConfig(), IpReassemblyConfig ipReassemblyCfg = IpReassemblyConfig(),
             TcpReassemblyConfig tcpReassemblyCfg = TcpReassemblyConfig());

    /**
     * @brief 添加应用层解析器(当协议端口不固定或者未知时使用此接口)
     * @param parser 应用层协议解析器
     * @return true-添加成功, false-添加失败
     */
    bool addProtocolParser(const std::shared_ptr<ProtocolParser>& parser);

    /**
     * @brief 添加应用层解析器(当协议端口固定且已知时使用此接口)
     * @param port 端口号
     * @param parser 应用层协议解析器
     * @return true-添加成功, false-添加失败
     */
    bool addProtocolParser(uint16_t port, const std::shared_ptr<ProtocolParser>& parser);

    /**
     * @brief 删除应用层解析器
     * @param protocol 应用层协议
     */
    void removeProtocolParser(uint32_t protocol);

    /**
     * @brief 解析数据(注意: 非线程安全, 不可跨线程调用)
     * @param num 数据序号, 自定义(一般是递增)
     * @param data 数据
     * @param dataLen 数据长度
     * @param dataSource 数据源
     * @return -1-数据为空, 0-成功, 1-解析以太网层失败, 2-解析网络层失败, 3-解析传输层失败, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片), 6-达到最大递归深度
     */
    int parse(size_t num, const uint8_t* data, uint32_t dataLen, const DataSource& dataSource = DataSource::NETWORK_ETH);

private:
    /**
     * @brief 分片缓存信息
     */
    struct FragmentInfo
    {
        std::chrono::steady_clock::time_point lastAccessTime; /* 最近访问时间(用于超时和LRU) */
        uint8_t originalProtocol = 0; /* 原始协议类型(IPv6需要) */
        std::map<uint32_t, std::vector<uint8_t>> fragments; /* 分片集合(key-偏移量, value-分片数据) */
        bool gotLastFragment = false; /* 是否已收到最后一片 */
        uint32_t lastOffset = 0; /* 最后一片的偏移量 */
        uint32_t totalLen = 0; /* 重组后总长度 */
        uint32_t totalPayloadSize = 0; /* 已缓存分片负载总大小(非重组后长度) */
        size_t fragmentCount = 0; /* 分片数量计数 */
    };

    /**
     * @brief IPv6分片头部信息
     */
    struct Ipv6FragmentHeader
    {
        uint8_t originalProtocol = 0; /* 分片头中的原始协议 */
        bool isMoreFragment = false; /* M标志 */
        uint32_t fragOffset = 0; /* 分片偏移 */
        uint32_t fragHeaderLen = 0; /* 分片头长度(固定8) */
        uint32_t identification = 0; /* 分片ID */
    };

    /**
     * @brief TCP段信息
     */
    struct TcpSegment
    {
        uint32_t seq = 0; /* TCP序列号 */
        uint32_t payloadLen = 0; /* 负载长度 */
        std::vector<uint8_t> data; /* 数据 */
        std::chrono::steady_clock::time_point recvTime; /* 接收时间 */
        bool isFin = false; /* 是否是FIN包 */
        bool isRst = false; /* 是否是RST包 */
    };

    /**
     * @brief TCP流重组信息
     */
    struct TcpStreamInfo
    {
        std::chrono::steady_clock::time_point lastAccessTime; /* 最近访问时间(用于超时和LRU) */
        uint32_t nextExpectedSeq = 0; /* 期望的下一个序列号 */
        bool isSeqInitialized = false; /* 序列号是否已初始化(收到第一个SYN或数据的SYN) */
        std::map<uint32_t, TcpSegment> segments; /* 乱序段缓存(按seq排序) */
        std::vector<uint8_t> reassembledData; /* 已重组的连续数据(等待应用层消费) */
        std::vector<std::weak_ptr<ProtocolParser>> wpWaitingParserList; /* 等待更多数据的协议解析器列表 */
        bool needMoreData = false; /* 是否有解析器需要更多数据 */
        bool finReceived = false; /* 是否已收到FIN标记 */
        std::chrono::steady_clock::time_point finRecvTime; /* FIN接收时间 */
        bool rstReceived = false; /* 是否已收到RST标记 */
    };

private:
    /**
     * @brief 递归解析重组后的数据包(防止栈溢出)
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param data 数据
     * @param dataLen 数据长度
     * @param dataSource 数据源
     * @param depth 递归深度(防止无限递归)
     * @return -1-数据为空, 0-成功, 1-解析以太网层失败, 2-解析网络层失败, 3-解析传输层失败, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片), 6-达到最大递归深度
     */
    int parseWithDepthControl(size_t num, const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen,
                              const DataSource& dataSource, int depth);

    /**
     * @brief 处理从网络层开始的数据(用于重组后的IP包)
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param data 数据（从IP头开始）
     * @param dataLen 数据长度
     * @param dataSource 数据源（REASSEMBLED_IPv4 或 REASSEMBLED_IPv6）
     * @param depth 递归深度
     * @return 0-成功, 2-解析网络层失败, 3-解析传输层失败, 5-分片重组中(等待后续分片)
     */
    int parseFromNetworkLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen,
                              const DataSource& dataSource, int depth);

    /**
     * @brief 处理以太网层数据
     * @param num 数据序号
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param ethHeader [输出]以太网II协议头部
     * @param headerLen [输出]协议头部长度
     * @param networkProtocol [输出]网络层协议类型
     * @return 指向实际类型的基类指针
     */
    ProtocolHeader* handleEthernetLayer(size_t num, const uint8_t* data, uint32_t dataLen, EthernetIIHeader& ethHeader, uint32_t& headerLen,
                                        uint32_t& networkProtocol);

    /**
     * @brief 处理网络层数据
     * @param num 数据序号
     * @param networkProtocol 网络层协议类型
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param ipv4Header [输出]Ipv4头部对象
     * @param ipv6Header [输出]Ipv6头部对象
     * @param arpHeader [输出]Arp头部对象
     * @param headerLen [输出]协议头部长度
     * @param transportProtocol [输出]传输层协议类型
     * @return 指向实际类型的基类指针
     */
    ProtocolHeader* handleNetworkLayer(size_t num, uint32_t networkProtocol, const uint8_t* data, uint32_t dataLen, Ipv4Header& ipv4Header,
                                       Ipv6Header& ipv6Header, ArpHeader& arpHeader, uint32_t& headerLen, uint32_t& transportProtocol);

    /**
     * @brief 处理传输层数据
     * @param num 数据序号
     * @param transportProtocol 传输层协议类型
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param tcpHeader [输出]Tcp头部对象
     * @param udpHeader [输出]Udp头部对象
     * @param icmpHeader [输出]Icmp头部对象
     * @param icmpv6Header [输出]Icmpv6头部对象
     * @param headerLen [输出]协议头部长度
     * @return 指向实际类型的基类指针
     */
    ProtocolHeader* handleTransportLayer(size_t num, uint32_t transportProtocol, const uint8_t* data, uint32_t dataLen,
                                         TcpHeader& tcpHeader, UdpHeader& udpHeader, IcmpHeader& icmpHeader, Icmpv6Header& icmpv6Header,
                                         uint32_t& headerLen);

    /**
     * @brief 处理应用层数据
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 传输层负载
     * @param payloadLen 传输层负载长度
     * @param tcpKey TCP流键
     * @param depth 递归深度(防止无限递归)
     * @return 0-成功, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片), 6-达到最大递归深度
     */
    int handleApplicationLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                               const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen,
                               const TcpStreamKey* tcpKey = nullptr, int depth = 0);

    /**
     * @brief 遍历IPv6扩展头链获取最终协议类型
     * @param data IPv6数据包起始位置
     * @param dataLen 数据长度
     * @param nextHeader [输入/输出]当前协议类型，遍历后返回最终协议
     * @param totalExtLen [输出]扩展头总长度
     * @param stopAtFragment 是否在遇到分片头时停止遍历
     * @param fragHeader [输出]分片头信息
     * @return true-成功, false-失败(数据不足或格式错误)
     */
    bool traverseIpv6Extension(const uint8_t* data, uint32_t dataLen, uint8_t& nextHeader, uint32_t& totalExtLen,
                               bool stopAtFragment = false, Ipv6FragmentHeader* fragHeader = nullptr);

    /**
     * @brief 清理IP分片缓存
     * @param ntp 当前时间点
     */
    void cleanupFragmentCache(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 检查并处理分片
     * @param ntp 当前时间点
     * @param networkHeader 网络层头部
     * @param data 当前分片数据(含IP头)
     * @param dataLen 数据长度
     * @param isFragment [输出]是否为分片报文, true-是分片报文, false-不是分片报文
     * @return 返回重组后的完整数据
     */
    std::shared_ptr<std::vector<uint8_t>> checkAndHandleFragment(const std::chrono::steady_clock::time_point& ntp,
                                                                 const ProtocolHeader* networkHeader, const uint8_t* data, uint32_t dataLen,
                                                                 bool& isFragment);

    /**
     * @brief 解析IPv6分片头部
     * @param header IPv6头部
     * @param data 数据
     * @param dataLen 数据长度
     * @param originalProtocol [输出]原始协议类型
     * @param isMoreFragment [输出]是否有更多分片
     * @param fragOffset [输出]分片偏移量
     * @param fragHeaderLen [输出]分片头部长度
     * @param identification [输出]标识符
     * @return true-成功, false-失败
     */
    bool parseIpv6FragmentHeader(const Ipv6Header* header, const uint8_t* data, uint32_t dataLen, uint8_t& originalProtocol,
                                 bool& isMoreFragment, uint32_t& fragOffset, uint32_t& fragHeaderLen, uint32_t& identification);

    /**
     * @brief 清理TCP流缓存
     * @param ntp 当前时间点
     */
    void cleanupTcpStreamCache(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 检查并处理TCP流
     * @param ntp 当前时间点
     * @param networkHeader 网络层头部
     * @param transportHeader 传输层头部
     * @param payload 负载数据
     * @param payloadLen 负载数据长度
     * @param key [输出]TCP流标识健
     * @param needMoreData [输出]是否需要更多数据
     * @return 返回重组后的完整数据
     */
    std::shared_ptr<std::vector<uint8_t>> checkAndHandleTcpReassembly(const std::chrono::steady_clock::time_point& ntp,
                                                                      const ProtocolHeader* networkHeader,
                                                                      const ProtocolHeader* transportHeader, const uint8_t* payload,
                                                                      uint32_t payloadLen, TcpStreamKey& key, bool& needMoreData);

private:
    const CallbackConfig m_cbCfg; /* 回调配置 */
    const IpReassemblyConfig m_ipReassemblyCfg; /* IP分片重组配置 */
    const TcpReassemblyConfig m_tcpReassemblyCfg; /* TCP分片重组配置 */

    std::unordered_map<FragmentKey, std::shared_ptr<FragmentInfo>> m_fragmentCache; /* IP分片缓存 */
    std::chrono::steady_clock::time_point m_lastCleanupTime = std::chrono::steady_clock::now(); /* 上次清理IP分片缓存时间 */

    std::unordered_map<TcpStreamKey, std::shared_ptr<TcpStreamInfo>> m_tcpStreamCache; /* TCP流缓存 */
    std::chrono::steady_clock::time_point m_lastTcpCleanupTime = std::chrono::steady_clock::now(); /* 上次清理TCP流缓存时间 */

    std::mutex m_mutexParserList;
    std::vector<std::shared_ptr<ProtocolParser>> m_applicationParserList; /* 应用层解析器列表 */
    std::unordered_map<uint16_t, std::shared_ptr<ProtocolParser>> m_applicationParserMap; /* 应用层解析器映射表, key-端口 */
};
} // namespace npacket
