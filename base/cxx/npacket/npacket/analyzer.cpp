#include "analyzer.h"

#include <algorithm>

#include "helper.h"

namespace npacket
{
/**
 * @brief 限制网络分析配置
 * @param cfg 外部定义的配置信息
 * @return 限制后的新配置
 */
NetworkConfig limitNetworkConfig(NetworkConfig cfg)
{
    /* 限制不超过5分钟, 超过5分钟的分片几乎不可能是正常的网络延 */
    if (cfg.fragTimeout < 1000 || cfg.fragTimeout > 300000)
    {
        cfg.fragTimeout = 1000;
    }
    /* 限制不超过超时实际的1/5, 平衡CPU开销和响应速度, 过长导致僵尸缓存清理不及时 */
    if (cfg.fragClearInterval < 100 || cfg.fragClearInterval > 60000 || cfg.fragClearInterval > cfg.fragTimeout)
    {
        cfg.fragClearInterval = cfg.fragTimeout / 5;
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

Analyzer::Analyzer(NetworkConfig networkCfg) : m_networkCfg(limitNetworkConfig(networkCfg)) {}

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
        if (m_applicationParserMap.end() != m_applicationParserMap.find(port))
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

int Analyzer::parse(const uint8_t* data, uint32_t dataLen, const DataSource& dataSource)
{
    return parseWithDepthControl(data, dataLen, dataSource, 0);
}

int Analyzer::parseWithDepthControl(const uint8_t* data, uint32_t dataLen, const DataSource& dataSource, int depth)
{
    auto ntp = std::chrono::steady_clock::now();
    cleanupFragmentCache(ntp); /* 清空超时分片缓存 */
    if (!data || 0 == dataLen)
    {
        return -1;
    }
    if (depth >= m_networkCfg.maxRecursionDepth) /* 防止深度分片攻击(DoS)导致的栈溢出 */
    {
        return 6;
    }
    uint32_t remainLen = dataLen, offset = 0;
    std::shared_ptr<ProtocolHeader> transportHeader = nullptr;
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
        auto ethernetHeader = handleEthernetLayer(data + offset, remainLen, headerLen, networkProtocol);
        if (!ethernetHeader)
        {
            return 1;
        }
        remainLen -= headerLen;
        offset += headerLen;
        if (ehternetLayerCb && !ehternetLayerCb(ntp, dataLen, ethernetHeader, data + offset, remainLen))
        {
            return 0;
        }
        /* step2. 解析网络层 */
        if (remainLen > 0)
        {
            auto networkHeader = handleNetworkLayer(networkProtocol, data + offset, remainLen, headerLen, transportProtocol);
            if (!networkHeader)
            {
                return 2;
            }
            /* step3. 检查并处理分片 */
            bool isFragment = false;
            auto reassembledData = checkAndHandleFragment(networkHeader, data + offset, remainLen, isFragment);
            if (isFragment)
            {
                if (reassembledData) /* 分片已重组完成, 使用重组后的数据继续解析 */
                {
                    return parseWithDepthControl(reassembledData->data(), reassembledData->size(), dataSource, depth + 1);
                }
                return 5; /* 分片未收齐, 等待后续 */
            }
            networkHeader->parent = ethernetHeader;
            remainLen -= headerLen;
            offset += headerLen;
            if (networkLayerCb && !networkLayerCb(ntp, dataLen, networkHeader, data + offset, remainLen))
            {
                return 0;
            }
            /* step4. 解析传输层 */
            if (remainLen > 0)
            {
                transportHeader = handleTransportLayer(transportProtocol, data + offset, remainLen, headerLen);
                if (!transportHeader)
                {
                    return 3;
                }
                transportHeader->parent = networkHeader;
                remainLen -= headerLen;
                offset += headerLen;
                if (transportLayerCb && !transportLayerCb(ntp, dataLen, transportHeader, data + offset, remainLen))
                {
                    return 0;
                }
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
        std::vector<std::shared_ptr<ProtocolParser>> applicationParserList;
        {
            std::lock_guard<std::mutex> locker(m_mutexParserList);
            applicationParserList = m_applicationParserList;
        }
        return handleApplicationLayer(ntp, dataLen, transportHeader, data + offset, remainLen, applicationParserList);
    }
    return 0;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleEthernetLayer(const uint8_t* data, uint32_t dataLen, uint32_t& headerLen,
                                                              uint32_t& networkProtocol)
{
    if (data && dataLen >= EthernetIIHeader::getMinLen())
    {
        auto header = Helper::loadEthernetIIHeader(*(RawEthernetIIHeader*)(data));
        headerLen = header->headerLen;
        networkProtocol = header->nextProtocol;
        return header;
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleNetworkLayer(uint32_t networkProtocol, const uint8_t* data, uint32_t dataLen,
                                                             uint32_t& headerLen, uint32_t& transportProtocol)
{
    if (data)
    {
        switch ((NetworkProtocol)networkProtocol)
        {
        case NetworkProtocol::IPv4:
            if (dataLen >= Ipv4Header::getMinLen())
            {
                auto header = Helper::loadIpv4Header(*(RawIpv4Header*)(data));
                headerLen = header->headerLen;
                transportProtocol = header->nextProtocol;
                return header;
            }
            break;
        case NetworkProtocol::ARP:
            if (dataLen >= ArpHeader::getMinLen())
            {
                auto header = Helper::loadArpHeader(*(RawArpHeader*)(data));
                headerLen = header->headerLen;
                transportProtocol = 0; /* ARP无传输层 */
                return header;
            }
            break;
        case NetworkProtocol::IPv6:
            if (dataLen >= Ipv6Header::getMinLen())
            {
                auto header = Helper::loadIpv6Header(*(RawIpv6Header*)(data));
                headerLen = header->headerLen;
                uint8_t nextHeader = header->nextHeader;
                uint32_t extLen = 0;
                if (traverseIpv6Extension(data, dataLen, nextHeader, extLen, false, nullptr))
                {
                    headerLen = Ipv6Header::getMinLen() + extLen;
                    transportProtocol = nextHeader;
                    return header;
                }
            }
            break;
        }
    }
    return nullptr;
}

std::shared_ptr<ProtocolHeader> Analyzer::handleTransportLayer(uint32_t transportProtocol, const uint8_t* data, uint32_t dataLen,
                                                               uint32_t& headerLen)
{
    if (data)
    {
        switch ((TransportProtocol)transportProtocol)
        {
        case TransportProtocol::TCP:
            if (dataLen >= TcpHeader::getMinLen())
            {
                auto header = Helper::loadTcpHeader(*(RawTcpHeader*)(data));
                headerLen = header->headerLen;
                return header;
            }
            break;
        case TransportProtocol::UDP:
            if (dataLen >= UdpHeader::getMinLen())
            {
                auto header = Helper::loadUdpHeader(*(RawUdpHeader*)(data));
                headerLen = header->headerLen;
                return header;
            }
            break;
        case TransportProtocol::ICMP:
            if (dataLen >= IcmpHeader::getMinLen())
            {
                auto header = Helper::loadIcmpHeader(*(RawIcmpHeader*)(data));
                headerLen = header->headerLen;
                return header;
            }
            break;
        case TransportProtocol::ICMPv6:
            if (dataLen >= Icmpv6Header::getMinLen())
            {
                auto header = Helper::loadIcmpv6Header(*(RawIcmpv6Header*)(data));
                headerLen = header->headerLen;
                return header;
            }
            break;
        }
    }
    return nullptr;
}

int Analyzer::handleApplicationLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                     const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen,
                                     const std::vector<std::shared_ptr<ProtocolParser>>& applicationParserList)
{
    /* 从传输层头部提取端口 */
    uint16_t srcPort = 0, dstPort = 0;
    if (header)
    {
        if (TransportProtocol::TCP == header->getProtocol())
        {
            auto tcpHeader = std::static_pointer_cast<TcpHeader>(header);
            if (tcpHeader)
            {
                srcPort = tcpHeader->srcPort;
                dstPort = tcpHeader->dstPort;
            }
        }
        else if (TransportProtocol::UDP == header->getProtocol())
        {
            auto udpHeader = std::static_pointer_cast<UdpHeader>(header);
            if (udpHeader)
            {
                srcPort = udpHeader->srcPort;
                dstPort = udpHeader->dstPort;
            }
        }
    }
    /* 根据端口查找解析器(端口优先级: 1.dstPort, 2.srcPort) */
    std::shared_ptr<ProtocolParser> portParser = nullptr;
    {
        std::lock_guard<std::mutex> locker(m_mutexParserMap);
        auto iter = m_applicationParserMap.find(dstPort);
        if (m_applicationParserMap.end() != iter)
        {
            portParser = iter->second;
        }
        else
        {
            iter = m_applicationParserMap.find(srcPort);
            if (m_applicationParserMap.end() != iter)
            {
                portParser = iter->second;
            }
        }
    }
    uint32_t offset = 0; /* 已消费的字节偏移 */
    std::shared_ptr<ProtocolParser> stickyParser = nullptr; /* 粘包优化, 记住成功的解析器 */
    while (offset < payloadLen)
    {
        uint32_t consumeLen = 0;
        ParseResult result = ParseResult::FAILURE;
        auto parser = stickyParser ? stickyParser : portParser; /* 优先级: 1.上次成功的, 2.端口映射的 */
        if (parser)
        {
            result = parser->parse(ntp, totalLen, header, payload + offset, payloadLen - offset, consumeLen);
        }
        if (ParseResult::FAILURE == result) /* 失败则回退遍历 */
        {
            for (const auto& p : applicationParserList)
            {
                if (p != parser) /* 跳过已尝试的 */
                {
                    result = p->parse(ntp, totalLen, header, payload + offset, payloadLen - offset, consumeLen);
                    if (ParseResult::FAILURE != result)
                    {
                        stickyParser = p; /* 记录成功解析器 */
                        break;
                    }
                }
            }
        }
        switch (result) /* 结果处理 */
        {
        case ParseResult::SUCCESS:
            if (0 == consumeLen || consumeLen > payloadLen - offset) /* 无效消费 */
            {
                return 4;
            }
            offset += consumeLen;
            break;
        case ParseResult::CONTINUE:
            return 5;
        case ParseResult::FAILURE:
            return offset > 0 ? 0 : 4; /* 偏移值大于0表示有成功解析过 */
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
    std::lock_guard<std::mutex> locker(m_mutexFragmentCache);
    if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastCleanupTime).count() <= m_networkCfg.fragClearInterval)
    {
        return;
    }
    m_lastCleanupTime = ntp;
    /* step1. 清理超时分片缓存 */
    for (auto iter = m_fragmentCache.begin(); m_fragmentCache.end() != iter;)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(ntp - iter->second->lastAccessTime).count() > m_networkCfg.fragTimeout)
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
    if (cacheSize > m_networkCfg.maxCacheCount)
    {
        auto needRemoveCount = cacheSize - m_networkCfg.maxCacheCount;
        if (needRemoveCount > 0) /* 仍超过最大数量, 按LRU删除最旧的 */
        {
            /* 收集条目并按最后访问时间排序 */
            std::vector<std::pair<FragmentKey, std::chrono::steady_clock::time_point>> entries;
            for (const auto& kv : m_fragmentCache)
            {
                entries.emplace_back(kv.first, kv.second->lastAccessTime);
            }
            std::partial_sort(entries.begin(), entries.begin() + needRemoveCount, entries.end(), [](const auto& a, const auto& b) {
                return (a.second < b.second); /* 越久未访问的越靠前 */
            });
            /* 删除最旧的条目 */
            for (size_t i = 0; i < needRemoveCount && i < entries.size(); ++i)
            {
                m_fragmentCache.erase(entries[i].first);
            }
        }
    }
}

std::shared_ptr<std::vector<uint8_t>> Analyzer::checkAndHandleFragment(const std::shared_ptr<ProtocolHeader>& networkHeader,
                                                                       const uint8_t* data, uint32_t dataLen, bool& isFragment)
{
    isFragment = false;
    if (!networkHeader || !data || 0 == dataLen)
    {
        return nullptr;
    }
    bool isIpv4 = false;
    FragmentKey key;
    uint32_t headerLen = 0;
    bool isMoreFragment = false;
    uint32_t fragOffset = 0;
    uint8_t originalProtocol = 0; /* IPv6需要保存原始协议 */
    if (NetworkProtocol::IPv4 == networkHeader->getProtocol()) /* IPv4 */
    {
        isIpv4 = true;
        auto ipv4Header = std::static_pointer_cast<Ipv4Header>(networkHeader);
        if (!(ipv4Header->flagMore > 0 || ipv4Header->fragOffset > 0)) /* 非分片报文 */
        {
            return nullptr;
        }
        isFragment = true;
        key = FragmentKey::createIpv4FragmentKey(ipv4Header->srcAddr, ipv4Header->dstAddr, ipv4Header->identification);
        headerLen = ipv4Header->headerLen;
        isMoreFragment = ipv4Header->flagMore;
        fragOffset = ipv4Header->fragOffset;
    }
    else if (NetworkProtocol::IPv6 == networkHeader->getProtocol()) /* IPv6 */
    {
        auto ipv6Header = std::static_pointer_cast<Ipv6Header>(networkHeader);
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
    if (payloadLen > m_networkCfg.maxFragSize) /* 检查单分片负载大小 */
    {
        return nullptr;
    }
    if (fragOffset > (m_networkCfg.maxReassembleSize / 8)) /* 检查分片偏移量计算是否越界 */
    {
        return nullptr;
    }
    if ((isMoreFragment && 0 == payloadLen) || payloadLen > 65535) /* 检查分片负载有效性 */
    {
        return nullptr;
    }
    uint64_t estimatedTotal = (uint64_t)fragOffset * 8 + payloadLen;
    if (estimatedTotal > m_networkCfg.maxReassembleSize
        || estimatedTotal > std::numeric_limits<uint32_t>::max()) /* 预检查总大小(防止整数溢出) */
    {
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> locker(m_mutexFragmentCache);
        /* 查找或创建分片缓存 */
        std::shared_ptr<FragmentInfo> info;
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
        if (info->fragmentCount >= m_networkCfg.maxFragmentCount) /* 检查分片数量(疑似DoS攻击) */
        {
            m_fragmentCache.erase(key);
            return nullptr;
        }
        if (info->totalPayloadSize + payloadLen > m_networkCfg.maxReassembleSize) /* 检查缓存总大小 */
        {
            m_fragmentCache.erase(key);
            return nullptr;
        }
        /* 严格检查分片重叠(按RFC 5722, IPv6禁止重叠分片) */
        uint32_t newStart = fragOffset * 8;
        uint32_t newEnd = newStart + payloadLen;
        for (auto& kv : info->fragments)
        {
            uint32_t existStart = kv.first * 8;
            uint32_t existEnd = existStart + kv.second.size();
            if (newStart < existEnd && newEnd > existStart) /* 任何重叠都视为攻击(根据RFC 5722，IPv6禁止重叠分片), 删除整个分片组 */
            {
                m_fragmentCache.erase(key);
                return nullptr;
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
        if (0 == info->totalLen || info->totalLen > m_networkCfg.maxReassembleSize) /* 验证总长度 */
        {
            return nullptr;
        }
        /* 重组数据 */
        auto reassembledData = std::make_shared<std::vector<uint8_t>>();
        reassembledData->reserve(headerLen + info->totalLen);
        reassembledData->insert(reassembledData->end(), data, data + headerLen); /* 先复制IP头部 */
        /* 检查分片连续性 */
        uint32_t currentPos = 0;
        for (auto& kv : info->fragments)
        {
            uint32_t expectedPos = kv.first;
            if (expectedPos != currentPos) /* 分片不连续 */
            {
                return nullptr;
            }
            reassembledData->insert(reassembledData->end(), kv.second.begin(), kv.second.end());
            currentPos += kv.second.size();
        }
        if (currentPos != info->totalLen) /* 验证重组结果 */
        {
            return nullptr;
        }
        if (info->fragments.size() != info->fragmentCount) /* 检查是否所有分片都已使用 */
        {
            return nullptr;
        }
        /* 更新IP头部中的长度字段 */
        if (isIpv4) /* IPv4 */
        {
            uint16_t newTotalLen = (uint16_t)(reassembledData->size());
            auto ipHeader = (RawIpv4Header*)(reassembledData->data());
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
            uint16_t newPayloadLen = (uint16_t)(reassembledData->size() - Ipv6Header::getMinLen());
            auto ipHeader = (RawIpv6Header*)(reassembledData->data());
            ipHeader->payloadLen[0] = ((newPayloadLen >> 8) & 0xFF);
            ipHeader->payloadLen[1] = (newPayloadLen & 0xFF);
            ipHeader->nextHeader = originalProtocol; /* 恢复原始协议 */
        }
        return reassembledData;
    }
    return nullptr;
}

bool Analyzer::parseIpv6FragmentHeader(const std::shared_ptr<Ipv6Header>& header, const uint8_t* data, uint32_t dataLen,
                                       uint8_t& originalProtocol, bool& isMoreFragment, uint32_t& fragOffset, uint32_t& fragHeaderLen,
                                       uint32_t& identification)
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
} // namespace npacket
