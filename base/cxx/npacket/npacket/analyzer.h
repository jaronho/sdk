#pragma once
#include <chrono>
#include <functional>
#include <mutex>
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
 * @brief 分析器
 */
class Analyzer
{
public:
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
     * @return -1-数据为空, 0-成功, 1-解析以太网层失败, 2-解析网络层失败, 3-解析传输层失败
     */
    int parse(const uint8_t* data, uint32_t dataLen);

private:
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

private:
    std::mutex m_mutexLayerCb;
    LAYER_CALLBACK m_ethernetLayerCb = nullptr; /* 以太网层数据回调 */
    LAYER_CALLBACK m_networkLayerCb = nullptr; /* 网络层数据回调 */
    LAYER_CALLBACK m_transportLayerCb = nullptr; /* 传输层数据回调 */
    std::mutex m_mutexParserList;
    std::vector<std::shared_ptr<ProtocolParser>> m_applicationParserList; /* 应用层解析器列表 */
};
} // namespace npacket
