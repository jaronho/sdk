#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <string.h>
#include <vector>

#include "protocol.h"
#include "protocol_parser.h"

namespace npacket
{
/**
 * @brief 层数据回调
 * @param ntp 数据包接收时间点
 * @param totalLen 数据包总长度
 * @param header 层头部
 * @param payload 层负载
 * @param payloadLen 层负载长度
 * @return true-继续处理下一层, false-停止后续处理
 */
using LAYER_CALLBACK = std::function<bool(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                          const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)>;

/**
 * @brief 数据源
 */
enum DataSource
{
    NETWORK = 0, /* 标准网络包 */
    SERIAL /* 串口数据包 */
};

/**
 * @brief 网络包分析配置
 */
struct NetworkConfig
{
    size_t fragTimeout = 30000; /* 分片重组超时时间(毫秒) */
    size_t fragClearInterval = 6000; /* 分片缓存清理间隔(毫秒) */
    size_t maxFragSize = 8192; /* 单个分片最大负载大小(防止大分片攻击) */
    size_t maxFragmentCount = 32; /* 每组分片最大数量(防止分片攻击) */
    size_t maxReassembleSize = 65535; /* 最大重组后数据包大小(防止分片攻击) */
    size_t maxCacheCount = 500; /* 最大分片缓存数量(防止分片攻击) */
    size_t maxRecursionDepth = 3; /* 最大递归深度(防止分片嵌套攻击) */
};

/**
 * @brief 分析器
 */
class Analyzer
{
public:
    /**
     * @brief 构造函数
     * @param networkCfg 网络包分析配置
     */
    Analyzer(NetworkConfig networkCfg = NetworkConfig());

    /**
     * @brief 设置层数据回调
     * @param ethernetLayerCb 以太网层数据回调
     * @param networkLayerCb 网络层数据回调
     * @param transportLayerCb 传输层数据回调
     */
    void setLayerCallback(const LAYER_CALLBACK& ethernetLayerCb, const LAYER_CALLBACK& networkLayerCb,
                          const LAYER_CALLBACK& transportLayerCb);

    /**
     * @brief 添加应用层解析器
     * @param parser 应用层协议解析器
     * @return true-添加成功, false-添加失败
     */
    bool addProtocolParser(const std::shared_ptr<ProtocolParser>& parser);

    /**
     * @brief 删除应用层解析器
     * @param protocol 应用层协议
     */
    void removeProtocolParser(uint32_t protocol);

    /**
     * @brief 解析数据
     * @param data 数据
     * @param dataLen 数据长度
     * @param dataSource 数据源
     * @return -1-数据为空, 0-成功, 1-解析以太网层失败, 2-解析网络层失败, 3-解析传输层失败, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片), 6-达到最大递归深度
     */
    int parse(const uint8_t* data, uint32_t dataLen, const DataSource& dataSource = DataSource::NETWORK);

private:
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
        static FragmentKey createIpv4FragmentKey(const uint8_t srcIp[4], const uint8_t dstIp[4], uint16_t identification)
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
        static FragmentKey createIpv6FragmentKey(const uint8_t srcIp[16], const uint8_t dstIp[16], uint32_t identification)
        {
            FragmentKey key;
            key.ipVersion = 6;
            memcpy(&key.v6.srcIp, srcIp, sizeof(key.v6.srcIp));
            memcpy(&key.v6.dstIp, dstIp, sizeof(key.v6.dstIp));
            key.v6.identification = identification;
            return key;
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

private:
    /**
     * @brief 递归解析重组后的数据包(防止栈溢出)
     * @param data 数据
     * @param dataLen 数据长度
     * @param dataSource 数据源
     * @param depth 递归深度(防止无限递归)
     * @return -1-数据为空, 0-成功, 1-解析以太网层失败, 2-解析网络层失败, 3-解析传输层失败, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片), 6-达到最大递归深度
     */
    int parseWithDepthControl(const uint8_t* data, uint32_t dataLen, const DataSource& dataSource, int depth);

    /**
     * @brief 处理以太网层数据
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param headerLen [输出]协议头部长度
     * @param networkProtocol [输出]网络层协议类型
     * @return 协议头部
     */
    std::shared_ptr<ProtocolHeader> handleEthernetLayer(const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                        uint32_t& networkProtocol);

    /**
     * @brief 处理网络层数据
     * @param networkProtocol 网络层协议类型
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param headerLen [输出]协议头部长度
     * @param transportProtocol [输出]传输层协议类型
     * @return 协议头部
     */
    std::shared_ptr<ProtocolHeader> handleNetworkLayer(uint32_t networkProtocol, const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                       uint32_t& transportProtocol);

    /**
     * @brief 处理传输层数据
     * @param transportProtocol 传输层协议类型
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param headerLen [输出]协议头部长度
     * @return 协议头部
     */
    std::shared_ptr<ProtocolHeader> handleTransportLayer(uint32_t transportProtocol, const uint8_t* data, uint32_t dataLen,
                                                         uint32_t& headerLen);

    /**
     * @brief 处理应用层数据
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 传输层负载
     * @param payloadLen 传输层负载长度
     * @param applicationParserList 应用层解析器列表
     * @return 0-成功, 4-无匹配的应用层解析器, 5-分片重组中(等待后续分片)
     */
    int handleApplicationLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                               const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen,
                               const std::vector<std::shared_ptr<ProtocolParser>>& applicationParserList);

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
     * @brief 清理分片缓存
     * @param ntp 当前时间点
     */
    void cleanupFragmentCache(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 检查并处理分片
     * @param networkHeader 网络层头部
     * @param data 当前分片数据(含IP头)
     * @param dataLen 数据长度
     * @param isFragment [输出]是否为分片报文, true-是分片报文, false-不是分片报文
     * @return 返回重组后的完整数据
     */
    std::shared_ptr<std::vector<uint8_t>> checkAndHandleFragment(const std::shared_ptr<ProtocolHeader>& networkHeader, const uint8_t* data,
                                                                 uint32_t dataLen, bool& isFragment);

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
    bool parseIpv6FragmentHeader(const std::shared_ptr<Ipv6Header>& header, const uint8_t* data, uint32_t dataLen,
                                 uint8_t& originalProtocol, bool& isMoreFragment, uint32_t& fragOffset, uint32_t& fragHeaderLen,
                                 uint32_t& identification);

private:
    const NetworkConfig m_networkCfg; /* 网络包分析配置 */

    std::mutex m_mutexLayerCb;
    LAYER_CALLBACK m_ethernetLayerCb = nullptr; /* 以太网层数据回调 */
    LAYER_CALLBACK m_networkLayerCb = nullptr; /* 网络层数据回调 */
    LAYER_CALLBACK m_transportLayerCb = nullptr; /* 传输层数据回调 */

    std::mutex m_mutexFragmentCache;
    std::map<FragmentKey, std::shared_ptr<FragmentInfo>> m_fragmentCache; /* 分片缓存 */
    std::chrono::steady_clock::time_point m_lastCleanupTime = std::chrono::steady_clock::now(); /* 上次清理分片缓存时间 */

    std::mutex m_mutexParserList;
    std::vector<std::shared_ptr<ProtocolParser>> m_applicationParserList; /* 应用层解析器列表 */
};
} // namespace npacket
