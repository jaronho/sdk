#pragma once
#include <functional>

#include "protocol.h"

namespace npacket
{
/**
 * @brief 层数据回调
 * @param totalLen 数据包总长度
 * @param header 层头部
 * @param payload 层负载
 * @param payloadLen 层负载长度
 * @return true-继续处理下一层, false-停止后续处理
 */
using LAYER_CALLBACK =
    std::function<bool(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen)>;

/**
 * @brief 包分析器
 */
class PacketAnalyzer
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
     * @brief 分析数据
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
     * @param networkProtocol [输出]传输层协议类型
     * @return 协议头部
     */
    std::shared_ptr<ProtocolHeader> handleNetworkLayer(const uint32_t& networkProtocol, const uint8_t* data, uint32_t dataLen,
                                                       uint32_t& headerLen, uint32_t& transportProtocol);

    /**
     * @brief 处理传输层数据
     * @param transportProtocol 传输层协议类型
     * @param data 层数据
     * @param dataLen 层数据长度
     * @param headerLen [输出]协议头部长度
     * @return 协议头部
     */
    std::shared_ptr<ProtocolHeader> handleTransportLayer(const uint32_t& transportProtocol, const uint8_t* data, uint32_t dataLen,
                                                         uint32_t& headerLen);

private:
    LAYER_CALLBACK m_ethernetLayerCb = nullptr; /* 以太网层数据回调 */
    LAYER_CALLBACK m_networkLayerCb = nullptr; /* 网络层数据回调 */
    LAYER_CALLBACK m_transportLayerCb = nullptr; /* 传输层数据回调 */
};
} // namespace npacket
