#pragma once
#include <functional>

#include "../protocol_parser.h"

namespace npacket
{
/**
 * @brief FTP协议解析器(https://www.rfc-editor.org/rfc/rfc959.html)
 */
class FtpParser : public ProtocolParser
{
public:
    /**
     * @brief 请求包回调
     * @param totalLen 数据包总长度
     * @param transportHeader 传输层头部
     * @param cmd 命令
     * @param param 命令参数
     */
    using REQUEST_CALLBACK = std::function<void(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader,
                                                const std::string& cmd, const std::string& param)>;

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
    bool parse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
               uint32_t payloadLen) override;

    /**
     * @brief 设置请求包回调
     * @param calback 回调
     */
    void setRequestCallback(const REQUEST_CALLBACK& callback);

private:
    /**
     * @brief 解析请求包
     */
    bool parseRequest(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                      uint32_t payloadLen);

    /**
     * @brief 解析响应包
     */
    bool parseResponse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& transportHeader, const uint8_t* payload,
                       uint32_t payloadLen);

private:
    REQUEST_CALLBACK m_requestCb = nullptr; /* 请求包回调 */
};
} // namespace npacket
