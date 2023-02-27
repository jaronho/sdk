#pragma once
#include "../parser.h"

namespace npacket
{
/**
 * @brief FTP协议解析器(https://www.rfc-editor.org/rfc/rfc959.html)
 */
class FtpParser : public ProtocolParser
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
     * @param transportHeader 传输层头部
     * @param payload 层负载
     * @param payloadLen 层负载长度
     * @return true-成功, false-失败
     */
    bool parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload, uint32_t payloadLen);
};
} // namespace npacket
