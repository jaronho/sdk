#pragma once
#include "protocol.h"

namespace npacket
{
/**
 * @brief 应用层协议
 */
enum ApplicationProtocol
{
    FTP,
};

/**
 * @brief 应用层协议解析器(接口类)
 */
class ProtocolParser
{
public:
    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    virtual uint32_t getProtocol() const = 0;

    /**
     * @brief 解析
     * @param totalLen 数据包总长度
     * @param transportHeader 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @return true-成功, false-失败
     */
    virtual bool parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                       uint32_t payloadLen) = 0;
};
} // namespace npacket
