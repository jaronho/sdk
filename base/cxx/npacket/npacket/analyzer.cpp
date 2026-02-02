#include "analyzer.h"

#include <algorithm>

#include "helper.h"

namespace npacket
{
/**
 * @brief 限制IP分片重组配置
 * @param cfg 外部定义的配置信息
 * @return 限制后的新配置
 */
IpReassemblyConfig limitIpReassemblyConfig(IpReassemblyConfig cfg)
{
    /* 限制不超过5分钟, 超过5分钟的分片几乎不可能是正常的网络延 */
    if (cfg.fragTimeout < 1000 || cfg.fragTimeout > 300000)
    {
        cfg.fragTimeout = 1000;
    }
    /* 限制不超过超时实际的1/5, 平衡CPU开销和响应速度, 过长导致僵尸缓存清理不及时 */
    if (cfg.fragClearInterval < 100 || cfg.fragClearInterval > 60000 || cfg.fragClearInterval > cfg.fragTimeout)
    {
        cfg.fragClearInterval = cfg.fragTimeout / 5 + 10;
        if (cfg.fragClearInterval < 3000) /* 保证至少3秒间隔, 避免频繁扫描 */
        {
            cfg.fragClearInterval = 3000;
        }
    }
    /* 限制单个报文不超过16MB(已远超常规应用层协议需求) */
    if (cfg.maxReassembleSize < 1280 || cfg.maxReassembleSize > 16777216) /* 1280-IPv6最小MTU */
    {
        cfg.maxReassembleSize = 65535;
    }
    /* 限制分片数不超过256(RFC 791建议值), 超过极不常见 */
    if (0 == cfg.maxFragmentCount || cfg.maxFragmentCount > 256)
    {
        cfg.maxFragmentCount = 32;
    }
    /* 限制单个分片不超过16KB, 超过此值攻击意图明显 */
    if (cfg.maxFragSize < 8 || cfg.maxFragSize > 16384 && cfg.maxFragSize > cfg.maxReassembleSize - Ipv6Header::getMinLen())
    {
        cfg.maxFragSize = 8192;
    }
    /* 限制缓存数量不超过5000条 */
    if (0 == cfg.maxCacheCount || cfg.maxCacheCount > 5000)
    {
        cfg.maxCacheCount = 1000;
    }
    /* 限制递归栈不超过5层(理论安全值), 超过风险急剧上升 */
    if (0 == cfg.maxRecursionDepth || cfg.maxRecursionDepth > 5)
    {
        cfg.maxRecursionDepth = 3;
    }
    return cfg;
}

/**
 * @brief 限制IP分片重组配置
 * @param cfg 外部定义的配置信息
 * @return 限制后的新配置
 */
TcpReassemblyConfig limitTcpReassemblyConfig(TcpReassemblyConfig cfg)
{
    if (cfg.streamTimeout < 1000 || cfg.streamTimeout > 300000)
    {
        cfg.streamTimeout = 60000;
    }
    if (cfg.streamClearInterval < 100 || cfg.streamClearInterval > 60000 || cfg.streamClearInterval > cfg.streamTimeout)
    {
        cfg.streamClearInterval = cfg.streamTimeout / 5 + 10;
        if (cfg.streamClearInterval < 5000) /* 保证至少5秒间隔, 避免频繁扫描 */
        {
            cfg.streamClearInterval = 5000;
        }
    }
    if (cfg.maxStreamSize < 65536 || cfg.maxStreamSize > 67108864) /* 1280-IPv6最小MTU */
    {
        cfg.maxStreamSize = 1048576;
    }
    if (0 == cfg.maxStreamCount || cfg.maxStreamCount > 100000)
    {
        cfg.maxStreamCount = 1000;
    }
    if (0 == cfg.maxSegmentsPerStream || cfg.maxSegmentsPerStream > 256)
    {
        cfg.maxSegmentsPerStream = 32; /* 默认32个, 超过将淘汰最旧的 */
    }
    if (cfg.finWaitTimeout < 1000 || cfg.finWaitTimeout > cfg.streamTimeout)
    {
        cfg.finWaitTimeout = 5000; /* 默认5秒, 最长不超过普通流超时 */
    }
    if (cfg.gapSizeThreshold > 1048576) /* 限制不超过1MB */
    {
        cfg.gapSizeThreshold = 1048576;
    }
    return cfg;
}

Analyzer::Analyzer(IpReassemblyConfig ipReassemblyCfg, TcpReassemblyConfig tcpReassemblyCfg)
    : m_ipReassemblyCfg(limitIpReassemblyConfig(ipReassemblyCfg)), m_tcpReassemblyCfg(limitTcpReassemblyConfig(tcpReassemblyCfg))
{
}

void Analyzer::setLayerCallback(const LAYER_CALLBACK& ethernetLayerCb, const LAYER_CALLBACK& networkLayerCb,
                                const LAYER_CALLBACK& transportLayerCb)
{
    std::lock_guard<std::mutex> locker(m_mutexLayerCb);
    m_ethernetLayerCb = ethernetLayerCb;
    m_networkLayerCb = networkLayerCb;
    m_transportLayerCb = transportLayerCb;
}

bool Analyzer::addProtocolParser(const std::shared_ptr<ProtocolParser>& parser)
{
    if (parser)
    {
        std::lock_guard<std::mutex> locker(m_mutexParserList);
        for (const auto& item : m_applicationParserList)
        {
            if (item && item->getProtocol() == parser->getProtocol())
            {
                return false;
            }
        }
        m_applicationParserList.emplace_back(parser);
        return true;
    }
    return false;
}

bool Analyzer::addProtocolParser(uint16_t port, const std::shared_ptr<ProtocolParser>& parser)
{
    addProtocolParser(parser); /* 保留遍历能力 */
    if (parser && port > 0)
    {
        std::lock_guard<std::mutex> locker(m_mutexParserMap);
        if (m_applicationParserMap.end() == m_applicationParserMap.find(port))
        {
            m_applicationParserMap.insert(std::make_pair(port, parser));
        }
        return true;
    }
    return false;
}

void Analyzer::removeProtocolParser(uint32_t protocol)
{
    {
        std::lock_guard<std::mutex> locker(m_mutexParserList);
        for (auto iter = m_applicationParserList.begin(); m_applicationParserList.end() != iter; ++iter)
        {
            if (*iter && (*iter)->getProtocol() == protocol)
            {
                m_applicationParserList.erase(iter);
                break;
            }
        }
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexParserMap);
        for (auto iter = m_applicationParserMap.begin(); m_applicationParserMap.end() != iter;)
        {
            if (iter->second && iter->second->getProtocol() == protocol)
            {
                iter = m_applicationParserMap.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

int Analyzer::parse(size_t num, const uint8_t* data, uint32_t dataLen, const DataSource& dataSource)
{
    auto ntp = std::chrono::steady_clock::now();
    return parseWithDepthControl(num, ntp, data, dataLen, dataSource, 0);
}

int Analyzer::parseWithDepthControl(size_t num, const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen,
                                    const DataSource& dataSource, int depth)
{
    cleanupFragmentCache(ntp); /* 清空超时IP分片缓存 */
    cleanupTcpStreamCache(ntp); /* 清理超时TCP流缓存 */
    if (!data || 0 == dataLen)
    {
        return -1;
    }
    if (depth >= m_ipReassemblyCfg.maxRecursionDepth) /* 防止深度分片攻击(DoS)导致的栈溢出 */
    {
        return 6;
    }
    uint32_t remainLen = dataLen, offset = 0;
    EthernetIIHeader ethHeader;
    Ipv4Header ipv4Header;
    Ipv6Header ipv6Header;
    ArpHeader arpHeader;
    TcpHeader tcpHeader;
    UdpHeader udpHeader;
    IcmpHeader icmpHeader;
    Icmpv6Header icmpv6Header;
    ProtocolHeader *ethernetHeader = nullptr, *networkHeader = nullptr, *transportHeader = nullptr;
    uint32_t appDataLen = remainLen;
    bool willParseApplication = false;
    if (DataSource::NETWORK == dataSource) /* 标准网络数据包 */
    {
        uint32_t headerLen = 0, networkProtocol = 0, transportProtocol = 0;
        LAYER_CALLBACK ehternetLayerCb = nullptr, networkLayerCb = nullptr, transportLayerCb = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutexLayerCb);
            ehternetLayerCb = m_ethernetLayerCb;
            networkLayerCb = m_networkLayerCb;
            transportLayerCb = m_transportLayerCb;
        }
        /* step1. 解析以太网层 */
        ethernetHeader = handleEthernetLayer(num, data + offset, remainLen, ethHeader, headerLen, networkProtocol);
        if (!ethernetHeader)
        {
            return 1;
        }
        remainLen -= headerLen;
        offset += headerLen;
        if (ehternetLayerCb && !ehternetLayerCb(num, ntp, dataLen, ethernetHeader, data + offset, remainLen))
        {
            return 0;
        }
        /* step2. 解析网络层 */
        if (remainLen > 0)
        {
            networkHeader = handleNetworkLayer(num, networkProtocol, data + offset, remainLen, ipv4Header, ipv6Header, arpHeader, headerLen,
                                               transportProtocol);
            if (!networkHeader)
            {
                return 2;
            }
            /* step3. 检查并处理分片 */
            bool isFragment = false;
            auto reassembledIp = checkAndHandleFragment(networkHeader, data + offset, remainLen, isFragment);
            if (isFragment)
            {
                if (reassembledIp) /* 分片已重组完成, 使用重组后的数据继续解析 */
                {
                    return parseWithDepthControl(num, ntp, reassembledIp->data(), reassembledIp->size(), dataSource, depth + 1);
                }
                return 5; /* 分片未收齐, 等待后续 */
            }
            networkHeader->parent = ethernetHeader;
            remainLen -= headerLen;
            offset += headerLen;
            if (networkLayerCb && !networkLayerCb(num, ntp, dataLen, networkHeader, data + offset, remainLen))
            {
                return 0;
            }
            /* step4. 解析传输层 */
            if (remainLen > 0)
            {
                transportHeader = handleTransportLayer(num, transportProtocol, data + offset, remainLen, tcpHeader, udpHeader, icmpHeader,
                                                       icmpv6Header, headerLen);
                if (!transportHeader)
                {
                    return 3;
                }
                transportHeader->parent = networkHeader;
                remainLen -= headerLen;
                offset += headerLen;
                uint32_t tcpPlayoadLen = remainLen; /* TCP负载长度, 根据IP层的totalLen裁剪TCP负载(Ethernet Padding), 默认使用剩余长度 */
                if (NetworkProtocol::IPv4 == networkProtocol)
                {
                    if (ipv4Header.totalLen >= ipv4Header.headerLen + headerLen) /* IPv4 totalLen 包含 IP头 + IP负载(TCP头 + TCP数据) */
                    {
                        /* 计算实际的TCP数据长度 = totalLen - IP头长度 - TCP头长度 */
                        tcpPlayoadLen = ipv4Header.totalLen - ipv4Header.headerLen - headerLen;
                        if (tcpPlayoadLen > remainLen) /* 防御性检查: tcpPlayoadLen 不应大于 remainLen */
                        {
                            tcpPlayoadLen = remainLen;
                        }
                    }
                    else /* 长度字段异常 */
                    {
                        tcpPlayoadLen = 0;
                    }
                }
                else if (NetworkProtocol::IPv6 == networkHeader->getProtocol())
                {
                    if (ipv6Header.payloadLen >= headerLen)
                    {
                        tcpPlayoadLen = ipv6Header.payloadLen - headerLen; /* headerLen应包含TCP头+Extension Headers */
                    }
                    else /* 长度字段异常 */
                    {
                        tcpPlayoadLen = 0;
                    }
                }
                if (transportLayerCb && !transportLayerCb(num, ntp, dataLen, transportHeader, data + offset, tcpPlayoadLen))
                {
                    return 0;
                }
                appDataLen = tcpPlayoadLen;
                willParseApplication = true;
            }
        }
    }
    else if (DataSource::SERIAL == dataSource) /* 串口数据包 */
    {
        willParseApplication = true;
    }
    /* 解析应用层 */
    if (willParseApplication)
    {
        const uint8_t* appData = data + offset;
        /* 如果是TCP, 进行TCP流重组 */
        if (transportHeader && TransportProtocol::TCP == transportHeader->getProtocol())
        {
            TcpStreamKey tcpKey;
            bool needMoreData = false;
            auto reassembledTcp = checkAndHandleTcpReassembly(networkHeader, transportHeader, appData, appDataLen, tcpKey, needMoreData);
            if (reassembledTcp && !reassembledTcp->empty()) /* 有重组后的数据, 解析应用层, 并传入tcpKey用于关联状态 */
            {
                return handleApplicationLayer(num, ntp, dataLen, transportHeader, reassembledTcp->data(), reassembledTcp->size(), &tcpKey,
                                              depth);
            }
            else if (needMoreData) /* TCP流在等待更多数据, 返回CONTINUE */
            {
                return 5;
            }
            /* 流已关闭或出错, 继续尝试解析当前包(可能是RST/FIN) */
        }
        /* 非TCP或无需重组, 直接解析 */
        return handleApplicationLayer(num, ntp, dataLen, transportHeader, appData, appDataLen, nullptr, depth);
    }
    return 0;
}

ProtocolHeader* Analyzer::handleEthernetLayer(size_t num, const uint8_t* data, uint32_t dataLen, EthernetIIHeader& ethHeader,
                                              uint32_t& headerLen, uint32_t& networkProtocol)
{
    if (data && dataLen >= EthernetIIHeader::getMinLen())
    {
        Helper::loadEthernetIIHeader(*(RawEthernetIIHeader*)(data), ethHeader);
        headerLen = ethHeader.headerLen;
        networkProtocol = ethHeader.nextProtocol;
        return &ethHeader;
    }
    return nullptr;
}

ProtocolHeader* Analyzer::handleNetworkLayer(size_t num, uint32_t networkProtocol, const uint8_t* data, uint32_t dataLen,
                                             Ipv4Header& ipv4Header, Ipv6Header& ipv6Header, ArpHeader& arpHeader, uint32_t& headerLen,
                                             uint32_t& transportProtocol)
{
    if (data)
    {
        switch ((NetworkProtocol)networkProtocol)
        {
        case NetworkProtocol::IPv4:
            if (dataLen >= Ipv4Header::getMinLen())
            {
                Helper::loadIpv4Header(*(RawIpv4Header*)(data), ipv4Header);
                headerLen = ipv4Header.headerLen;
                transportProtocol = ipv4Header.nextProtocol;
                return &ipv4Header;
            }
            break;
        case NetworkProtocol::ARP:
            if (dataLen >= ArpHeader::getMinLen())
            {
                Helper::loadArpHeader(*(RawArpHeader*)(data), arpHeader);
                headerLen = arpHeader.headerLen;
                transportProtocol = 0; /* ARP无传输层 */
                return &arpHeader;
            }
            break;
        case NetworkProtocol::IPv6:
            if (dataLen >= Ipv6Header::getMinLen())
            {
                Helper::loadIpv6Header(*(RawIpv6Header*)(data), ipv6Header);
                headerLen = ipv6Header.headerLen;
                uint8_t nextHeader = ipv6Header.nextHeader;
                uint32_t extLen = 0;
                if (traverseIpv6Extension(data, dataLen, nextHeader, extLen, false, nullptr))
                {
                    headerLen = Ipv6Header::getMinLen() + extLen;
                    transportProtocol = nextHeader;
                    return &ipv6Header;
                }
            }
            break;
        }
    }
    return nullptr;
}

ProtocolHeader* Analyzer::handleTransportLayer(size_t num, uint32_t transportProtocol, const uint8_t* data, uint32_t dataLen,
                                               TcpHeader& tcpHeader, UdpHeader& udpHeader, IcmpHeader& icmpHeader,
                                               Icmpv6Header& icmpv6Header, uint32_t& headerLen)
{
    if (data)
    {
        switch ((TransportProtocol)transportProtocol)
        {
        case TransportProtocol::TCP:
            if (dataLen >= TcpHeader::getMinLen())
            {
                Helper::loadTcpHeader(*(RawTcpHeader*)(data), tcpHeader);
                headerLen = tcpHeader.headerLen;
                return &tcpHeader;
            }
            break;
        case TransportProtocol::UDP:
            if (dataLen >= UdpHeader::getMinLen())
            {
                Helper::loadUdpHeader(*(RawUdpHeader*)(data), udpHeader);
                headerLen = udpHeader.headerLen;
                return &udpHeader;
            }
            break;
        case TransportProtocol::ICMP:
            if (dataLen >= IcmpHeader::getMinLen())
            {
                Helper::loadIcmpHeader(*(RawIcmpHeader*)(data), icmpHeader);
                headerLen = icmpHeader.headerLen;
                return &icmpHeader;
            }
            break;
        case TransportProtocol::ICMPv6:
            if (dataLen >= Icmpv6Header::getMinLen())
            {
                Helper::loadIcmpv6Header(*(RawIcmpv6Header*)(data), icmpv6Header);
                headerLen = icmpv6Header.headerLen;
                return &icmpv6Header;
            }
            break;
        }
    }
    return nullptr;
}

int Analyzer::handleApplicationLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                     const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen, const TcpStreamKey* tcpKey,
                                     int depth)
{
    if (depth >= m_ipReassemblyCfg.maxRecursionDepth) /* 防止递归深度攻击(如嵌套封装协议) */
    {
        return 6; /* 达到最大递归深度 */
    }
    if (!payload || 0 == payloadLen)
    {
        return 4;
    }
    /* 从传输层头部提取端口 */
    uint16_t srcPort = 0, dstPort = 0;
    if (header)
    {
        if (TransportProtocol::TCP == header->getProtocol())
        {
            auto tcpHeader = (const TcpHeader*)(header);
            if (tcpHeader)
            {
                srcPort = tcpHeader->srcPort;
                dstPort = tcpHeader->dstPort;
            }
        }
        else if (TransportProtocol::UDP == header->getProtocol())
        {
            auto udpHeader = (const UdpHeader*)(header);
            if (udpHeader)
            {
                srcPort = udpHeader->srcPort;
                dstPort = udpHeader->dstPort;
            }
        }
    }
    /* 获取候选解析器 */
    std::vector<std::shared_ptr<ProtocolParser>> candidateParsers;
    {
        /* 1. 端口匹配的解析器优先(端口优先级: 1.dstPort, 2.srcPort) */
        std::lock_guard<std::mutex> locker(m_mutexParserMap);
        auto iter = m_applicationParserMap.find(dstPort);
        if (m_applicationParserMap.end() != iter)
        {
            candidateParsers.emplace_back(iter->second);
        }
        else
        {
            iter = m_applicationParserMap.find(srcPort);
            if (m_applicationParserMap.end() != iter)
            {
                candidateParsers.emplace_back(iter->second);
            }
        }
    }
    {
        /* 2. 全局解析器列表(排除已添加的端口匹配协议) */
        std::lock_guard<std::mutex> locker(m_mutexParserList);
        for (const auto& parser : m_applicationParserList)
        {
            bool alreadyAdded = false;
            for (const auto& existing : candidateParsers)
            {
                if (existing == parser || (existing && parser && existing->getProtocol() == parser->getProtocol()))
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded)
            {
                candidateParsers.push_back(parser);
            }
        }
    }
    /* 准备解析数据(可能是合并后的) */
    std::vector<uint8_t> combinedData;
    const uint8_t* fullData = payload;
    uint32_t fullDataLen = payloadLen;
    std::shared_ptr<TcpStreamInfo> streamInfo = nullptr;
    if (tcpKey)
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
        auto iter = m_tcpStreamCache.find(*tcpKey);
        if (m_tcpStreamCache.end() != iter)
        {
            streamInfo = iter->second;
            if (streamInfo)
            {
                /* 如果之前有缓存的未消费数据, 与新数据合并 */
                if (!streamInfo->reassembledData.empty())
                {
                    /* 合并历史数据 + 新数据 */
                    combinedData.reserve(streamInfo->reassembledData.size() + payloadLen);
                    combinedData.insert(combinedData.end(), std::make_move_iterator(streamInfo->reassembledData.begin()),
                                        std::make_move_iterator(streamInfo->reassembledData.end()));
                    combinedData.insert(combinedData.end(), payload, payload + payloadLen);
                    fullData = combinedData.data();
                    fullDataLen = combinedData.size();
                    /* 清空已合并的缓存, 避免重复处理 */
                    streamInfo->reassembledData.clear();
                }
                /* 如果有之前等待数据的协议, 优先尝试它们 */
                if (!streamInfo->waitingParsers.empty())
                {
                    /* 将等待的解析器移到最前面 */
                    std::vector<std::shared_ptr<ProtocolParser>> temp;
                    for (const auto& wp : streamInfo->waitingParsers)
                    {
                        /* 检查是否仍在全局列表中(未被删除) */
                        bool stillValid = false;
                        for (const auto& cp : candidateParsers)
                        {
                            if (cp == wp)
                            {
                                stillValid = true;
                                temp.push_back(wp);
                                break;
                            }
                        }
                    }
                    /* 移除已失效的等待解析器 */
                    streamInfo->waitingParsers = temp;
                    /* 重组候选列表: 等待的优先, 然后是其他 */
                    std::vector<std::shared_ptr<ProtocolParser>> newCandidates = streamInfo->waitingParsers;
                    for (const auto& cp : candidateParsers)
                    {
                        bool isWaiting = false;
                        for (const auto& wp : streamInfo->waitingParsers)
                        {
                            if (cp == wp)
                            {
                                isWaiting = true;
                                break;
                            }
                        }
                        if (!isWaiting)
                        {
                            newCandidates.push_back(cp);
                        }
                    }
                    candidateParsers = std::move(newCandidates);
                }
            }
        }
    }
    /* 协议解析 */
    uint32_t offset = 0; /* 已消费的字节偏移 */
    std::vector<std::shared_ptr<ProtocolParser>> continueParsers; /* 返回CONTINUE的解析器 */
    std::shared_ptr<ProtocolParser> successParser;
    uint32_t successConsumeLen = 0;
    while (offset < fullDataLen)
    {
        continueParsers.clear();
        bool anySuccess = false;
        for (const auto& parser : candidateParsers) /* 尝试所有候选解析器 */
        {
            uint32_t consumeLen = 0;
            ParseResult result = parser->parse(num, ntp, totalLen, header, fullData + offset, fullDataLen - offset, consumeLen);
            if (ParseResult::SUCCESS == result)
            {
                if (consumeLen > 0 && consumeLen <= fullDataLen - offset)
                {
                    successParser = parser;
                    successConsumeLen = consumeLen;
                    anySuccess = true;
                    break; /* 找到成功的, 跳出循环 */
                }
            }
            else if (ParseResult::CONTINUE == result)
            {
                continueParsers.push_back(parser);
            }
            /* FAILURE, 继续尝试下一个 */
        }
        if (anySuccess) /* 成功处理, 更新状态 */
        {
            offset += successConsumeLen;
            /* 如果之前有等待的解析器, 清空(因为成功了) */
            if (streamInfo)
            {
                std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
                /* 重新查找流信息, 确保流仍然存在 */
                auto iter = m_tcpStreamCache.find(*tcpKey);
                if (m_tcpStreamCache.end() != iter && iter->second == streamInfo)
                {
                    streamInfo->waitingParsers.clear();
                    streamInfo->needMoreData = false;
                }
            }
            continue;
        }
        else /* 没有解析器成功 */
        {
            if (!continueParsers.empty()) /* 有解析器需要更多数据 */
            {
                if (streamInfo && tcpKey)
                {
                    std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
                    /* 重新查找流信息, 确保流仍然存在 */
                    auto iter = m_tcpStreamCache.find(*tcpKey);
                    if (m_tcpStreamCache.end() != iter && iter->second == streamInfo)
                    {
                        streamInfo->waitingParsers = continueParsers;
                        streamInfo->needMoreData = true;
                        /* 保存未消费的数据(相对于合并后的数据) */
                        std::vector<uint8_t> newUnconsumedData(fullData + offset, fullData + fullDataLen);
                        if (streamInfo->reassembledData.empty())
                        {
                            streamInfo->reassembledData = std::move(newUnconsumedData);
                        }
                        else
                        {
                            /* 先检查合并后大小, 避免超过限制 */
                            size_t totalSize = streamInfo->reassembledData.size() + newUnconsumedData.size();
                            if (totalSize > m_tcpReassemblyCfg.maxStreamSize)
                            {
                                /* 保留最新的数据(尾部) */
                                size_t excess = totalSize - m_tcpReassemblyCfg.maxStreamSize;
                                if (excess >= streamInfo->reassembledData.size())
                                {
                                    /* 旧数据全部废弃, 使用新数据 */
                                    streamInfo->reassembledData = std::move(newUnconsumedData);
                                    /* 调整期望序列号, 标记有数据丢失 */
                                    if (streamInfo->nextExpectedSeq > excess)
                                    {
                                        streamInfo->nextExpectedSeq -= (streamInfo->reassembledData.size() - excess);
                                    }
                                }
                                else /* 删除旧数据的头部, 腾出空间给新数据 */
                                {
                                    streamInfo->reassembledData.erase(streamInfo->reassembledData.begin(),
                                                                      streamInfo->reassembledData.begin() + excess);
                                    streamInfo->reassembledData.insert(streamInfo->reassembledData.end(), newUnconsumedData.begin(),
                                                                       newUnconsumedData.end());
                                }
                            }
                            else
                            {
                                streamInfo->reassembledData.insert(streamInfo->reassembledData.end(), newUnconsumedData.begin(),
                                                                   newUnconsumedData.end());
                            }
                        }
                    }
                }
                return 5; /* CONTINUE, 等待更多数据 */
            }
            else /* 所有解析器都失败 */
            {
                if (streamInfo && tcpKey)
                {
                    std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
                    auto iter = m_tcpStreamCache.find(*tcpKey);
                    if (m_tcpStreamCache.end() != iter && iter->second == streamInfo)
                    {
                        streamInfo->waitingParsers.clear();
                        streamInfo->needMoreData = false;
                    }
                }
                return (offset > 0 ? 0 : 4); /* 部分成功或完全失败 */
            }
        }
    }
    /* 所有数据处理完毕 */
    if (streamInfo && tcpKey)
    {
        std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
        auto iter = m_tcpStreamCache.find(*tcpKey);
        if (m_tcpStreamCache.end() != iter && iter->second == streamInfo)
        {
            streamInfo->waitingParsers.clear();
            streamInfo->needMoreData = false;
        }
    }
    return 0;
}

bool Analyzer::traverseIpv6Extension(const uint8_t* data, uint32_t dataLen, uint8_t& nextHeader, uint32_t& totalExtLen, bool stopAtFragment,
                                     Ipv6FragmentHeader* fragHeader)
{
    totalExtLen = 0;
    if (!data || dataLen < Ipv6Header::getMinLen() + 8)
    {
        return false;
    }
    const uint8_t* currentPtr = data + Ipv6Header::getMinLen();
    uint32_t remainLen = dataLen - Ipv6Header::getMinLen();
    while (remainLen >= 8) /* 遍历扩展头链查找分片头, 至少8字节(分片头大小) */
    {
        switch (nextHeader)
        {
        case IPV6_EXT_FRAGMENT: { /* 找到分片头, 停止遍历 */
            if (remainLen < 8)
            {
                return false;
            }
            if (stopAtFragment && fragHeader) /* 解析分片头 */
            {
                fragHeader->originalProtocol = currentPtr[0]; /* 暂存原始协议 */
                uint16_t fragOffsetRaw = ((currentPtr[2] << 8) | currentPtr[3]);
                fragHeader->isMoreFragment = (0 != (fragOffsetRaw & 0x0001)); /* 提取M标志 */
                fragHeader->fragOffset = (fragOffsetRaw & 0x1FFF); /* 提取13位偏移值 */
                fragHeader->fragHeaderLen = 8; /* 分片头固定8字节 */
                fragHeader->identification = ((currentPtr[4] << 24) | (currentPtr[5] << 16) | (currentPtr[6] << 8) | currentPtr[7]);
            }
            totalExtLen += 8;
            return true;
        }
        case IPV6_EXT_HOP_BY_HOP:
        case IPV6_EXT_ROUTING:
        case IPV6_EXT_DEST_OPTIONS: {
            if (remainLen < 2)
            {
                return false;
            }
            uint8_t extLen = currentPtr[1];
            if (extLen > (std::numeric_limits<uint32_t>::max() / 8 - 1)) /* 防止溢出 */
            {
                return false;
            }
            uint32_t headerSize = (uint32_t)(extLen + 1) * 8;
            if (remainLen < headerSize)
            {
                return false;
            }
            nextHeader = currentPtr[0];
            totalExtLen += headerSize;
            currentPtr += headerSize;
            remainLen -= headerSize;
            break;
        }
        case IPV6_EXT_ESP: /* 无法解析长度, 分片头不可能在其后 */
        case IPV6_EXT_AUTH: /* 同样无法安全解析, RFC 4302规定AH头长度可变, 但分片头不允许出现在其后 */
            return (stopAtFragment ? false : true); /* 根据调用者需求决定 */
        case IPV6_EXT_NO_NEXT: /* 无后续头 */
            return true;
        default: /* 非扩展头, 遍历完成 */
            return true;
        }
    }
    return true;
}

void Analyzer::cleanupFragmentCache(const std::chrono::steady_clock::time_point& ntp)
{
    if (!m_ipReassemblyCfg.enable)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(m_mutexFragmentCache);
    if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastCleanupTime).count() <= m_ipReassemblyCfg.fragClearInterval)
    {
        return;
    }
    m_lastCleanupTime = ntp;
    /* step1. 清理超时分片缓存 */
    for (auto iter = m_fragmentCache.begin(); m_fragmentCache.end() != iter;)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - iter->second->lastAccessTime).count()
            > m_ipReassemblyCfg.fragTimeout)
        {
            iter = m_fragmentCache.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    /* step2. 限制分片缓存大小 */
    auto cacheSize = m_fragmentCache.size();
    if (cacheSize > m_ipReassemblyCfg.maxCacheCount)
    {
        auto needRemoveCount = cacheSize - m_ipReassemblyCfg.maxCacheCount;
        /* 收集条目 */
        std::vector<std::pair<FragmentKey, std::chrono::steady_clock::time_point>> entries;
        entries.reserve(cacheSize);
        for (const auto& kv : m_fragmentCache)
        {
            entries.emplace_back(kv.first, kv.second->lastAccessTime);
        }
        std::nth_element(entries.begin(), entries.begin() + needRemoveCount, entries.end(),
                         [](const auto& a, const auto& b) { return a.second < b.second; });
        /* 删除最旧的条目 */
        for (size_t i = 0; i < needRemoveCount && i < entries.size(); ++i)
        {
            m_fragmentCache.erase(entries[i].first);
        }
    }
}

std::shared_ptr<std::vector<uint8_t>> Analyzer::checkAndHandleFragment(const ProtocolHeader* networkHeader, const uint8_t* data,
                                                                       uint32_t dataLen, bool& isFragment)
{
    isFragment = false;
    if (!networkHeader || !data || 0 == dataLen)
    {
        return nullptr;
    }
    if (!m_ipReassemblyCfg.enable)
    {
        return nullptr;
    }
    bool isIpv4 = false;
    FragmentKey key;
    uint32_t headerLen = 0;
    uint32_t basicHeaderLen = 0; /* 基本IP头长度(不包含扩展头) */
    bool isMoreFragment = false;
    uint32_t fragOffset = 0;
    uint8_t originalProtocol = 0; /* IPv6需要保存原始协议 */
    if (NetworkProtocol::IPv4 == networkHeader->getProtocol()) /* IPv4 */
    {
        isIpv4 = true;
        auto ipv4Header = (const Ipv4Header*)(networkHeader);
        if (!(ipv4Header->flagMore > 0 || ipv4Header->fragOffset > 0)) /* 非分片报文 */
        {
            return nullptr;
        }
        isFragment = true;
        key = FragmentKey::createIpv4FragmentKey(ipv4Header->srcAddr, ipv4Header->dstAddr, ipv4Header->identification);
        headerLen = ipv4Header->headerLen;
        basicHeaderLen = headerLen; /* IPv4没有扩展头概念 */
        isMoreFragment = ipv4Header->flagMore;
        fragOffset = ipv4Header->fragOffset;
    }
    else if (NetworkProtocol::IPv6 == networkHeader->getProtocol()) /* IPv6 */
    {
        auto ipv6Header = (const Ipv6Header*)(networkHeader);
        uint32_t fragHeaderLen = 0, identification = 0;
        if (!parseIpv6FragmentHeader(ipv6Header, data, dataLen, originalProtocol, isMoreFragment, fragOffset, fragHeaderLen,
                                     identification)) /* 非分片报文 */
        {
            return nullptr;
        }
        isFragment = true;
        uint8_t srcBytes[16], dstBytes[16];
        for (int i = 0; i < 8; ++i)
        {
            srcBytes[i * 2] = (ipv6Header->srcAddr[i] >> 8);
            srcBytes[i * 2 + 1] = (ipv6Header->srcAddr[i] & 0xFF);
            dstBytes[i * 2] = (ipv6Header->dstAddr[i] >> 8);
            dstBytes[i * 2 + 1] = (ipv6Header->dstAddr[i] & 0xFF);
        }
        key = FragmentKey::createIpv6FragmentKey(srcBytes, dstBytes, identification);
        headerLen = Ipv6Header::getMinLen() + fragHeaderLen; /* 基本头 + 扩展头(含分片头) */
        basicHeaderLen = Ipv6Header::getMinLen(); /* 仅基本头40字节 */
    }
    else /* 非IP协议, 不处理分片 */
    {
        return nullptr;
    }
    if (headerLen > dataLen) /* 头部长度不合理 */
    {
        return nullptr;
    }
    if (fragOffset > (std::numeric_limits<uint16_t>::max() / 8)) /* 偏移量过大, 非法分片 */
    {
        return nullptr;
    }
    const uint8_t* payload = data + headerLen;
    uint32_t payloadLen = dataLen - headerLen;
    if (payloadLen > m_ipReassemblyCfg.maxFragSize) /* 检查单分片负载大小 */
    {
        return nullptr;
    }
    if (fragOffset > (m_ipReassemblyCfg.maxReassembleSize / 8)) /* 检查分片偏移量计算是否越界 */
    {
        return nullptr;
    }
    if ((isMoreFragment && 0 == payloadLen) || payloadLen > 65535) /* 检查分片负载有效性 */
    {
        return nullptr;
    }
    uint64_t estimatedTotal = (uint64_t)fragOffset * 8 + payloadLen;
    if (estimatedTotal > m_ipReassemblyCfg.maxReassembleSize
        || estimatedTotal > std::numeric_limits<uint32_t>::max()) /* 预检查总大小(防止整数溢出) */
    {
        return nullptr;
    }
    {
        std::shared_ptr<FragmentInfo> info;
        std::lock_guard<std::mutex> locker(m_mutexFragmentCache);
        /* 查找或创建分片缓存 */
        auto iter = m_fragmentCache.find(key);
        if (m_fragmentCache.end() == iter)
        {
            info = std::make_shared<FragmentInfo>();
            info->originalProtocol = originalProtocol; /* 保存原始协议 */
            m_fragmentCache.insert(std::make_pair(key, info));
        }
        else
        {
            info = iter->second;
        }
        info->lastAccessTime = std::chrono::steady_clock::now(); /* 更新访问时间 */
        if (info->fragmentCount >= m_ipReassemblyCfg.maxFragmentCount) /* 检查分片数量(疑似DoS攻击) */
        {
            m_fragmentCache.erase(key);
            return nullptr;
        }
        if (info->totalPayloadSize + payloadLen > m_ipReassemblyCfg.maxReassembleSize) /* 检查缓存总大小 */
        {
            m_fragmentCache.erase(key);
            return nullptr;
        }
        /* 分片重叠处理 */
        uint32_t newStart = fragOffset * 8;
        uint32_t newEnd = newStart + payloadLen;
        if (isIpv4) /* IPv4: RFC 791 允许重叠分片, 采用"后到的覆盖先到的"策略处理四种重叠场景 */
        {
            for (auto iterFrag = info->fragments.begin(); iterFrag != info->fragments.end();)
            {
                uint32_t existStart = iterFrag->first * 8;
                uint32_t existEnd = existStart + iterFrag->second.size();
                /* 场景0: 无重叠(新分片完全在前或在后) */
                if (newStart >= existEnd || newEnd <= existStart)
                {
                    ++iterFrag;
                    continue;
                }
                /* 场景1: 新分片完全覆盖旧分片(新 < 旧 且 新 > 旧尾) */
                if (newStart <= existStart && newEnd >= existEnd)
                {
                    info->totalPayloadSize -= iterFrag->second.size();
                    iterFrag = info->fragments.erase(iterFrag);
                }
                /* 场景2: 新分片内嵌在旧分片内部(旧 < 新 且 旧尾 > 新尾) */
                else if (existStart < newStart && existEnd > newEnd)
                {
                    uint32_t frontLen = newStart - existStart; /* 前段保留长度 */
                    uint32_t backStart = newEnd; /* 后段起始字节偏移 */
                    uint32_t backLen = existEnd - newEnd; /* 后段保留长度 */
                    /* 保存旧数据引用 */
                    std::vector<uint8_t> oldData = std::move(iterFrag->second);
                    /* 删除原条目(迭代器失效, 需要重新获取) */
                    iterFrag = info->fragments.erase(iterFrag);
                    info->totalPayloadSize -= oldData.size();
                    /* 保存前段(如果存在) - 偏移量不变 */
                    if (frontLen > 0)
                    {
                        uint32_t frontOffset = existStart / 8;
                        info->fragments[frontOffset] = std::vector<uint8_t>(oldData.begin(), oldData.begin() + frontLen);
                        info->totalPayloadSize += frontLen;
                    }
                    /* 保存后段(如果存在) - 偏移量需要重新计算 */
                    if (backLen > 0)
                    {
                        uint32_t backOffset = (backStart + 7) / 8; /* 向上取整到8字节块 */
                        info->fragments[backOffset] = std::vector<uint8_t>(oldData.begin() + (backStart - existStart), oldData.end());
                        info->totalPayloadSize += backLen;
                    }
                    /* 注意: 内嵌场景后, 当前旧分片已被分割, 继续检查下一个旧分片 */
                }
                /* 场景3: 尾部重叠(新分片头部与旧分片尾部重叠) */
                else if (existStart < newStart && existEnd <= newEnd && existEnd > newStart)
                {
                    uint32_t keepLen = newStart - existStart; /* 旧分片保留长度 */
                    iterFrag->second.resize(keepLen);
                    info->totalPayloadSize -= (existEnd - existStart - keepLen);
                    ++iterFrag;
                }
                /* 场景4: 头部重叠(新分片尾部与旧分片头部重叠) */
                else if (existStart >= newStart && existEnd > newEnd && existStart < newEnd)
                {
                    uint32_t skipLen = newEnd - existStart; /* 需要跳过的重复字节数 */
                    uint32_t newOffset = newEnd / 8; /* 新偏移量(按8字节对齐) */

                    std::vector<uint8_t> newData(iterFrag->second.begin() + skipLen, iterFrag->second.end());
                    info->totalPayloadSize -= skipLen;

                    iterFrag = info->fragments.erase(iterFrag);
                    info->fragments[newOffset] = std::move(newData);
                }
                else
                {
                    /* 其他边界情况(理论上不应到达) */
                    ++iterFrag;
                }
            }
        }
        else /* IPv6: RFC 5722 严格禁止重叠分片, 发现任何重叠立即废弃整个分片组 */
        {
            for (auto& kv : info->fragments)
            {
                uint32_t existStart = kv.first * 8;
                uint32_t existEnd = existStart + kv.second.size();
                if (newStart < existEnd && newEnd > existStart) /* 任何重叠都视为攻击 */
                {
                    m_fragmentCache.erase(key);
                    return nullptr;
                }
            }
        }
        /* 存储分片数据(跳过IP头, 只存payload) */
        info->fragments[fragOffset] = std::vector<uint8_t>(payload, payload + payloadLen);
        info->totalPayloadSize += payloadLen;
        ++info->fragmentCount;
        if (!isMoreFragment) /* 处理最后一个分片 */
        {
            info->gotLastFragment = true;
            info->lastOffset = fragOffset;
            info->totalLen = (uint32_t)estimatedTotal;
        }
        if (!info->gotLastFragment) /* 检查是否收齐所有分片, 继续等待 */
        {
            return nullptr;
        }
        m_fragmentCache.erase(key); /* 清理缓存 */
        if (0 == info->totalLen || info->totalLen > m_ipReassemblyCfg.maxReassembleSize) /* 验证总长度 */
        {
            return nullptr;
        }
        /* 重组数据 */
        auto reassembledIp = std::make_shared<std::vector<uint8_t>>();
        reassembledIp->reserve(basicHeaderLen + info->totalLen);
        reassembledIp->insert(reassembledIp->end(), data, data + basicHeaderLen); /* 先复制IP头部 */
        /* 检查分片连续性 */
        uint32_t currentPos = 0;
        for (auto& kv : info->fragments)
        {
            uint32_t expectedBytePos = kv.first * 8; /* 将块索引转换为字节偏移 */
            if (expectedBytePos != currentPos) /* 分片不连续 */
            {
                return nullptr;
            }
            reassembledIp->insert(reassembledIp->end(), kv.second.begin(), kv.second.end());
            currentPos += kv.second.size();
        }
        if (currentPos != info->totalLen) /* 验证重组结果 */
        {
            return nullptr;
        }
        /* 更新IP头部中的长度字段 */
        if (isIpv4) /* IPv4 */
        {
            uint16_t newTotalLen = (uint16_t)(reassembledIp->size());
            auto ipHeader = (RawIpv4Header*)(reassembledIp->data());
            ipHeader->totalLen[0] = ((newTotalLen >> 8) & 0xFF);
            ipHeader->totalLen[1] = (newTotalLen & 0xFF);
            /* 清除MF标志和片段偏移 */
            ipHeader->flags_offset[0] &= 0xE0; /* 清除MF和偏移 */
            ipHeader->flags_offset[1] = 0; /* 清除偏移低位 */
        }
        else /* IPv6 */
        {
            if (0 == info->originalProtocol) /* 原始协议无效 */
            {
                return nullptr;
            }
            uint16_t newPayloadLen = (uint16_t)(reassembledIp->size() - Ipv6Header::getMinLen());
            auto ipHeader = (RawIpv6Header*)(reassembledIp->data());
            ipHeader->payloadLen[0] = ((newPayloadLen >> 8) & 0xFF);
            ipHeader->payloadLen[1] = (newPayloadLen & 0xFF);
            ipHeader->nextHeader = originalProtocol; /* 恢复原始协议 */
        }
        return reassembledIp;
    }
    return nullptr;
}

bool Analyzer::parseIpv6FragmentHeader(const Ipv6Header* header, const uint8_t* data, uint32_t dataLen, uint8_t& originalProtocol,
                                       bool& isMoreFragment, uint32_t& fragOffset, uint32_t& fragHeaderLen, uint32_t& identification)
{
    if (!header || !data || dataLen < Ipv6Header::getMinLen() + 8)
    {
        return false;
    }
    uint8_t nextHeader = header->nextHeader;
    uint32_t totalExtLen = 0;
    Ipv6FragmentHeader fragInfo;
    if (traverseIpv6Extension(data, dataLen, nextHeader, totalExtLen, true, &fragInfo))
    {
        originalProtocol = fragInfo.originalProtocol;
        isMoreFragment = fragInfo.isMoreFragment;
        fragOffset = fragInfo.fragOffset;
        fragHeaderLen = fragInfo.fragHeaderLen;
        identification = fragInfo.identification;
        return true;
    }
    return false;
}

std::shared_ptr<std::vector<uint8_t>> Analyzer::checkAndHandleTcpReassembly(const ProtocolHeader* networkHeader,
                                                                            const ProtocolHeader* transportHeader, const uint8_t* payload,
                                                                            uint32_t payloadLen, TcpStreamKey& outKey, bool& needMoreData)
{
    needMoreData = false;
    if (!networkHeader || !transportHeader || !payload) /* 注意: 不能payloadLen为0就返回 */
    {
        return nullptr;
    }
    if (!m_tcpReassemblyCfg.enable)
    {
        return nullptr;
    }
    const TcpHeader* tcpHeader = (const TcpHeader*)(transportHeader);
    /* 构建TCP流键(四元组) */
    if (NetworkProtocol::IPv4 == networkHeader->getProtocol())
    {
        const Ipv4Header* ipv4 = (const Ipv4Header*)(networkHeader);
        outKey = TcpStreamKey::createIpv4(ipv4->srcAddr, ipv4->dstAddr, tcpHeader->srcPort, tcpHeader->dstPort);
    }
    else if (NetworkProtocol::IPv6 == networkHeader->getProtocol())
    {
        const Ipv6Header* ipv6 = (const Ipv6Header*)(networkHeader);
        uint8_t srcBytes[16], dstBytes[16];
        for (int i = 0; i < 8; ++i)
        {
            srcBytes[i * 2] = ipv6->srcAddr[i] >> 8;
            srcBytes[i * 2 + 1] = ipv6->srcAddr[i] & 0xFF;
            dstBytes[i * 2] = ipv6->dstAddr[i] >> 8;
            dstBytes[i * 2 + 1] = ipv6->dstAddr[i] & 0xFF;
        }
        outKey = TcpStreamKey::createIpv6(srcBytes, dstBytes, tcpHeader->srcPort, tcpHeader->dstPort);
    }
    else
    {
        return nullptr;
    }
    std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
    /* 获取或创建流信息 */
    std::shared_ptr<TcpStreamInfo> streamInfo = nullptr;
    auto iter = m_tcpStreamCache.find(outKey);
    if (m_tcpStreamCache.end() == iter) /* 新流 */
    {
        /* RST或FIN-ACK重传到达时, 流可能已被清理, 直接返回当前数据(如果有) */
        if (payloadLen > 0 && (tcpHeader->flagRst || tcpHeader->flagFin))
        {
            return std::make_shared<std::vector<uint8_t>>(payload, payload + payloadLen);
        }
        streamInfo = std::make_shared<TcpStreamInfo>();
        streamInfo->lastAccessTime = std::chrono::steady_clock::now();
        iter = m_tcpStreamCache.insert(std::make_pair(outKey, streamInfo)).first;
    }
    else /* 旧流 */
    {
        streamInfo = iter->second;
        streamInfo->lastAccessTime = std::chrono::steady_clock::now();
        if (!streamInfo) /* 防御性检查: 防止map中存在空指针 */
        {
            m_tcpStreamCache.erase(iter);
            return nullptr;
        }
        /* 如果流之前被标记为半关闭, 且有残留数据, 优先返回 */
        if (streamInfo->finReceived && !streamInfo->reassembledData.empty())
        {
            auto result = std::make_shared<std::vector<uint8_t>>();
            result->swap(streamInfo->reassembledData);
            /* 追加当前包数据(如果是期望的序列号) */
            if (payloadLen > 0 && tcpHeader->seq == streamInfo->nextExpectedSeq)
            {
                result->insert(result->end(), payload, payload + payloadLen);
                streamInfo->nextExpectedSeq += payloadLen;
            }
            /* 如果还有乱序数据无法重组, 立即尝试一次尽力重组 */
            if (!streamInfo->segments.empty())
            {
                uint32_t currentSeq = streamInfo->nextExpectedSeq;
                std::vector<uint8_t> additionalData;
                while (!streamInfo->segments.empty())
                {
                    auto segIter = streamInfo->segments.begin();
                    if (segIter->first > currentSeq + m_tcpReassemblyCfg.maxStreamSize
                        || segIter->first + segIter->second.data.size() < segIter->first) /* 防御性检查: 防止序列号回绕/溢出 */
                    {
                        streamInfo->segments.erase(segIter);
                        continue;
                    }
                    if (segIter->first == currentSeq)
                    {
                        additionalData.insert(additionalData.end(), segIter->second.data.begin(), segIter->second.data.end());
                        currentSeq = segIter->first + segIter->second.data.size();
                        streamInfo->segments.erase(segIter);
                    }
                    else if (segIter->first < currentSeq)
                    {
                        /* 旧段, 检查是否有重叠部分 */
                        uint32_t offset = currentSeq - segIter->first;
                        if (offset < segIter->second.data.size())
                        {
                            additionalData.insert(additionalData.end(), segIter->second.data.begin() + offset, segIter->second.data.end());
                            currentSeq = segIter->first + segIter->second.data.size();
                        }
                        streamInfo->segments.erase(segIter);
                    }
                    else
                    {
                        break;
                    }
                }
                if (!additionalData.empty())
                {
                    result->insert(result->end(), additionalData.begin(), additionalData.end());
                    streamInfo->nextExpectedSeq = currentSeq;
                }
            }
            /* 检查是否可以彻底删除流(无剩余数据且在FIN超时时间内) */
            if (streamInfo->segments.empty() && streamInfo->reassembledData.empty())
            {
                m_tcpStreamCache.erase(iter);
            }
            return result;
        }
    }
    if (tcpHeader->flagRst) /* 处理特殊控制位RST: 立即清空流, 但返回当前数据(如果有) */
    {
        auto result = std::make_shared<std::vector<uint8_t>>();
        if (!streamInfo->reassembledData.empty())
        {
            result->swap(streamInfo->reassembledData);
        }
        if (payloadLen > 0)
        {
            result->insert(result->end(), payload, payload + payloadLen);
        }
        m_tcpStreamCache.erase(iter);
        return result;
    }
    if (tcpHeader->flagFin) /* 处理FIN: 进入半关闭状态, 不立即删除流 */
    {
        auto result = std::make_shared<std::vector<uint8_t>>();
        /* 合并已重组数据 */
        if (!streamInfo->reassembledData.empty())
        {
            result->swap(streamInfo->reassembledData);
        }
        /* 追加当前FIN包数据 */
        if (payloadLen > 0 && (result->empty() || tcpHeader->seq == streamInfo->nextExpectedSeq))
        {
            result->insert(result->end(), payload, payload + payloadLen);
            if (streamInfo->isSeqInitialized)
            {
                streamInfo->nextExpectedSeq += payloadLen;
            }
        }
        /* 标记半关闭状态 */
        streamInfo->finReceived = true;
        streamInfo->finRecvTime = std::chrono::steady_clock::now();
        /* 立即尽力合并现有乱序数据 */
        if (!streamInfo->segments.empty())
        {
            uint32_t currentSeq = streamInfo->nextExpectedSeq;
            while (!streamInfo->segments.empty())
            {
                auto segIter = streamInfo->segments.begin();
                if (segIter->first > currentSeq + m_tcpReassemblyCfg.maxStreamSize
                    || segIter->first + segIter->second.data.size() < segIter->first) /* 防御性检查: 防止序列号异常 */
                {
                    streamInfo->segments.erase(segIter);
                    continue;
                }
                if (segIter->first <= currentSeq)
                {
                    uint32_t offset = 0;
                    if (segIter->first < currentSeq)
                    {
                        offset = currentSeq - segIter->first;
                        if (offset >= segIter->second.data.size()) /* 再次检查offset有效性 */
                        {
                            streamInfo->segments.erase(segIter);
                            continue;
                        }
                    }
                    result->insert(result->end(), segIter->second.data.begin() + offset, segIter->second.data.end());
                    currentSeq = segIter->first + segIter->second.data.size();
                    streamInfo->segments.erase(segIter);
                }
                else
                {
                    break;
                }
            }
            streamInfo->nextExpectedSeq = currentSeq;
        }
        /* 如果结果为空且无残留乱序数据, 立即删除, 否则保留流 */
        if (result->empty() && streamInfo->segments.empty())
        {
            m_tcpStreamCache.erase(iter);
        }
        else
        {
            streamInfo->lastAccessTime = std::chrono::steady_clock::now();
        }
        return result;
    }
    if (!streamInfo->isSeqInitialized) /* 初始化序列号 */
    {
        uint32_t initialSeq = tcpHeader->seq;
        if (tcpHeader->flagSyn)
        {
            initialSeq += 1;
        }
        streamInfo->nextExpectedSeq = initialSeq + payloadLen;
        streamInfo->isSeqInitialized = true;
        if (payloadLen > 0)
        {
            return std::make_shared<std::vector<uint8_t>>(payload, payload + payloadLen);
        }
        if (tcpHeader->flagSyn && !tcpHeader->flagFin && !tcpHeader->flagRst) /* 空负载时, 仅在SYN包场景下(连接建立中)才需要等待更多数据 */
        {
            needMoreData = true;
        }
        return nullptr;
    }
    uint32_t seq = tcpHeader->seq;
    if (seq < streamInfo->nextExpectedSeq) /* 序列号重传/重叠处理 */
    {
        /* 重复包或旧数据 */
        if (seq + payloadLen <= streamInfo->nextExpectedSeq)
        {
            /* 完全重复 */
            if (streamInfo->reassembledData.empty())
            {
                if (payloadLen > 0)
                {
                    needMoreData = true;
                }
                /* 空负载的重复包(如重复ACK)不需要等待更多数据 */
                return nullptr;
            }
            auto result = std::make_shared<std::vector<uint8_t>>();
            result->swap(streamInfo->reassembledData);
            return result;
        }
        /* 部分重叠, 截取新数据 */
        uint32_t offset = streamInfo->nextExpectedSeq - seq;
        if (offset < payloadLen)
        {
            payload += offset;
            payloadLen -= offset;
            seq = streamInfo->nextExpectedSeq;
        }
        else
        {
            if (payloadLen > 0) /* payloadLen > 0 但 offset >= payloadLen, 整个载荷都是旧的重复数据 */
            {
                needMoreData = true;
            }
            return nullptr;
        }
    }
    if (seq == streamInfo->nextExpectedSeq && payloadLen > 0) /* 按序到达 */
    {
        streamInfo->nextExpectedSeq = seq + payloadLen;
        auto result = std::make_shared<std::vector<uint8_t>>(payload, payload + payloadLen);
        /* 整合乱序段 */
        while (!streamInfo->segments.empty())
        {
            auto nextIter = streamInfo->segments.begin();
            if (nextIter->first > streamInfo->nextExpectedSeq + m_tcpReassemblyCfg.maxStreamSize
                || nextIter->first + nextIter->second.data.size() < nextIter->first) /* 防御性检查: 序列号有效性 */
            {
                streamInfo->segments.erase(nextIter);
                continue;
            }
            if (nextIter->first <= streamInfo->nextExpectedSeq)
            {
                if (nextIter->first + nextIter->second.data.size() > streamInfo->nextExpectedSeq)
                {
                    uint32_t offset = streamInfo->nextExpectedSeq - nextIter->first;
                    if (offset < nextIter->second.data.size())
                    {
                        result->insert(result->end(), nextIter->second.data.begin() + offset, nextIter->second.data.end());
                        streamInfo->nextExpectedSeq = nextIter->first + nextIter->second.data.size();
                    }
                }
                streamInfo->segments.erase(nextIter);
            }
            else
            {
                break;
            }
        }
        /* 流大小限制与解析器通知 */
        if (result->size() > m_tcpReassemblyCfg.maxStreamSize)
        {
            /* 截断前强制通知所有等待的解析器, 防止内存泄露 */
            if (!streamInfo->waitingParsers.empty())
            {
                for (auto& parser : streamInfo->waitingParsers)
                {
                    if (parser)
                    {
                        parser->reset(); /* 强制重置, 释放内部缓存 */
                    }
                }
                streamInfo->waitingParsers.clear();
                streamInfo->needMoreData = false;
            }
            size_t excess = result->size() - m_tcpReassemblyCfg.maxStreamSize;
            if (excess < result->size())
            {
                /* 保留尾部数据到流缓存, 头部数据返回给应用层 */
                std::vector<uint8_t> excessData(result->begin() + m_tcpReassemblyCfg.maxStreamSize, result->end());
                result->resize(m_tcpReassemblyCfg.maxStreamSize);
                streamInfo->reassembledData = std::move(excessData);
                streamInfo->nextExpectedSeq = seq + m_tcpReassemblyCfg.maxStreamSize;
            }
            else /* 异常情况, 直接清空避免崩溃 */
            {
                result->clear();
                streamInfo->reassembledData.clear();
            }
        }
        return result;
    }
    /* 乱序到达, 缓存 */
    if (payloadLen > 0)
    {
        /* 检查单流总数据量(已重组 + 乱序缓存 + 新数据) */
        size_t currentCachedSize = streamInfo->reassembledData.size();
        for (const auto& seg : streamInfo->segments)
        {
            if (currentCachedSize > SIZE_MAX - seg.second.data.size())
            {
                break;
            }
            currentCachedSize += seg.second.data.size();
        }
        /* 如果添加新数据会超过限制, 先淘汰最旧的乱序段 */
        while (currentCachedSize + payloadLen > m_tcpReassemblyCfg.maxStreamSize && !streamInfo->segments.empty())
        {
            auto oldest = streamInfo->segments.begin();
            currentCachedSize -= oldest->second.data.size();
            streamInfo->segments.erase(oldest);
        }
        /* 检查段数量限制 */
        if (streamInfo->segments.size() >= m_tcpReassemblyCfg.maxSegmentsPerStream)
        {
            auto oldest = streamInfo->segments.begin();
            currentCachedSize -= oldest->second.data.size();
            streamInfo->segments.erase(oldest);
        }
        if (payloadLen > m_tcpReassemblyCfg.maxStreamSize) /* 单个报文超过流限制, 视为异常攻击或畸形数据, 直接丢弃 */
        {
            m_tcpStreamCache.erase(iter);
            return nullptr;
        }
        /* 存储当前乱序段 */
        TcpSegment segment;
        segment.seq = seq;
        segment.payloadLen = payloadLen;
        segment.data.assign(payload, payload + payloadLen);
        segment.recvTime = std::chrono::steady_clock::now();
        streamInfo->segments[seq] = std::move(segment);
        /* 双阈值策略: 尽力交付 + 流重置 */
        if (m_tcpReassemblyCfg.gapSizeThreshold > 0 && !streamInfo->segments.empty())
        {
            uint32_t firstCachedSeq = streamInfo->segments.begin()->first;
            /* 检测到空缺: 缓存的最早数据与期望值之间存在差距 */
            if (firstCachedSeq > streamInfo->nextExpectedSeq)
            {
                uint32_t gapSize = firstCachedSeq - streamInfo->nextExpectedSeq;
                if (gapSize <= m_tcpReassemblyCfg.gapSizeThreshold) /* 小空缺(<=阈值): 尽力交付, 跳过缺失部分 */
                {
                    /* 跳过缺失部分, 从最早可用数据开始 */
                    streamInfo->nextExpectedSeq = firstCachedSeq;
                    /* 构造返回数据: 从最早段开始交付 */
                    auto result = std::make_shared<std::vector<uint8_t>>();
                    auto firstIter = streamInfo->segments.begin();
                    result->insert(result->end(), firstIter->second.data.begin(), firstIter->second.data.end());
                    streamInfo->nextExpectedSeq = firstIter->first + firstIter->second.data.size();
                    streamInfo->segments.erase(firstIter);
                    /* 继续整合后续可能连续的乱序段 */
                    while (!streamInfo->segments.empty())
                    {
                        auto nextIter = streamInfo->segments.begin();
                        if (nextIter->first <= streamInfo->nextExpectedSeq)
                        {
                            uint32_t overlap = 0;
                            if (nextIter->first < streamInfo->nextExpectedSeq)
                            {
                                overlap = streamInfo->nextExpectedSeq - nextIter->first;
                            }
                            if (overlap < nextIter->second.data.size())
                            {
                                result->insert(result->end(), nextIter->second.data.begin() + overlap, nextIter->second.data.end());
                                streamInfo->nextExpectedSeq = nextIter->first + nextIter->second.data.size();
                            }
                            streamInfo->segments.erase(nextIter);
                        }
                        else
                        {
                            break;
                        }
                    }
                    return result;
                }
                else /* 大空缺(>阈值): 流重置, 视为新连接 */
                {
                    /* 完全重置流状态 */
                    streamInfo->isSeqInitialized = false;
                    streamInfo->nextExpectedSeq = 0;
                    streamInfo->segments.clear();
                    streamInfo->reassembledData.clear();
                    streamInfo->waitingParsers.clear();
                    streamInfo->needMoreData = false;
                    streamInfo->finReceived = false;
                    streamInfo->rstReceived = false;
                    /* 将当前包视为新连接的第一个包重新初始化 */
                    uint32_t initialSeq = tcpHeader->seq;
                    if (tcpHeader->flagSyn)
                    {
                        initialSeq += 1;
                    }
                    streamInfo->nextExpectedSeq = initialSeq + payloadLen;
                    streamInfo->isSeqInitialized = true;
                    if (payloadLen > 0)
                    {
                        return std::make_shared<std::vector<uint8_t>>(payload, payload + payloadLen);
                    }
                    return nullptr;
                }
            }
        }
        needMoreData = true;
    }
    return nullptr;
}

void Analyzer::cleanupTcpStreamCache(const std::chrono::steady_clock::time_point& ntp)
{
    if (!m_tcpReassemblyCfg.enable)
    {
        return;
    }
    std::lock_guard<std::mutex> locker(m_mutexTcpStreamCache);
    if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastTcpCleanupTime).count() <= m_tcpReassemblyCfg.streamClearInterval)
    {
        return;
    }
    m_lastTcpCleanupTime = ntp;
    /* 清理超时的流 */
    for (auto iter = m_tcpStreamCache.begin(); iter != m_tcpStreamCache.end();)
    {
        auto& streamInfo = iter->second;
        /* FIN状态使用短超时, 正常状态使用标准超时 */
        size_t timeoutThreshold = streamInfo->finReceived ? m_tcpReassemblyCfg.finWaitTimeout : m_tcpReassemblyCfg.streamTimeout;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - streamInfo->lastAccessTime).count() > timeoutThreshold)
        {
            iter = m_tcpStreamCache.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    /* 限制流数量(LRU) */
    if (m_tcpStreamCache.size() > m_tcpReassemblyCfg.maxStreamCount)
    {
        auto needRemove = m_tcpStreamCache.size() - m_tcpReassemblyCfg.maxStreamCount;
        /* 收集条目 */
        std::vector<std::pair<TcpStreamKey, std::chrono::steady_clock::time_point>> entries;
        entries.reserve(m_tcpStreamCache.size());
        for (const auto& kv : m_tcpStreamCache)
        {
            entries.emplace_back(kv.first, kv.second->lastAccessTime);
        }
        std::nth_element(entries.begin(), entries.begin() + needRemove, entries.end(),
                         [](const auto& a, const auto& b) { return a.second < b.second; });
        /* 删除最旧的条目 */
        for (size_t i = 0; i < needRemove && i < entries.size(); ++i)
        {
            m_tcpStreamCache.erase(entries[i].first);
        }
    }
}
} // namespace npacket
