#pragma once
#include <functional>
#include <vector>

#include "../protocol_parser.h"
#include "modbus_def.h"

namespace npacket
{
/**
 * @brief Modbus/TCP协议解析器
 */
class ModbusTcpParser : public ProtocolParser
{
public:
    /**
     * @brief 数据回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param data Modbus数据
     */
    using DATA_CALLBACK = std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                             const ProtocolHeader* header, const std::shared_ptr<modbus::DataSt>& data)>;

public:
    /**
     * @brief 构造函数
     * @param portList 要监听的端口列表(默认502)
     */
    ModbusTcpParser(const std::vector<uint16_t>& portList = {502});

    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    uint32_t getProtocol() const override;

    /**
     * @brief 解析
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @param consumeLen [输出]消耗的长度
     * @return 解析结果
     */
    ParseResult parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                      const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

    /**
     * @brief 设置数据回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_CALLBACK& callback);

private:
    /**
     * @brief MBAP 头部结构(Modbus Application Protocol Header)
     */
    struct MbapHeader
    {
        uint16_t transactionId = 0; /* 事务标识符 */
        uint16_t protocolId = 0; /* 协议标识符(必须为0) */
        uint16_t length = 0; /* 长度 = 包括单元标识符 + 功能码 + 数据 */
        uint8_t unitId = 0; /* 单元标识符(从站地址) */
    };

    /**
     * @brief 解析MBAP头
     * @param data 数据
     * @param dataLen 数据长度
     * @param mbap [输出]MBAP头
     * @param headerLen [输出]MBAP头长度
     * @param pduLen [输出]PDU长度
     * @return true-成功, false-失败
     */
    bool parseMbapHeader(const uint8_t* data, uint32_t dataLen, MbapHeader& mbap, uint32_t& headerLen, uint32_t& pduLen);

    /**
     * @brief 验证端口是否匹配
     * @param port 端口
     * @return true-匹配, false-不匹配
     */
    bool isModbusPort(uint16_t port) const;

private:
    const std::vector<uint16_t> m_modbusPortList; /* Modbus端口列表 */

    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
};
} // namespace npacket
