#pragma once
#include <functional>

#include "../protocol_parser.h"

namespace npacket
{
/**
 * @brief IEC103协议解析器
 */
class Iec103Parser : public ProtocolParser
{
public:
    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    uint32_t getProtocol() const override;

    /**
     * @brief 解析
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @return true-成功, false-失败
     */
    bool parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen) override;

private:
    /**
     * @brief 解析固定帧
     */
    bool parseFixedFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析可变帧
     */
    bool parseVariableFrame(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 响应主控单元
     */
    void onMaster(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, uint8_t fcb, uint8_t fcv, uint8_t func,
                  const uint8_t* data, uint8_t dataLen);

    /**
     * @brief 响应保护设备
     */
    void onSlave(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, uint8_t acd, uint8_t dfc, uint8_t func,
                 const uint8_t* data, uint8_t dataLen);
};
} // namespace npacket
