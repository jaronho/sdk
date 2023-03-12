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
     * @brief 包回调
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param flag 命令/代码
     * @param arg 参数
     */
    using PKT_CALLBACK = std::function<void(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const std::string& flag,
                                            const std::string& arg)>;

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

    /**
     * @brief 设置请求包回调
     * @param reqCb 回调
     */
    void setRequestCallback(const PKT_CALLBACK& reqCb);

    /**
     * @brief 设置响应包回调
     * @param respCb 回调
     */
    void setResponseCallback(const PKT_CALLBACK& respCb);

private:
    /**
     * @brief 解析请求包
     */
    bool parseRequest(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析响应包
     */
    bool parseResponse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

private:
    PKT_CALLBACK m_requestCb = nullptr; /* 请求包回调 */
    PKT_CALLBACK m_responseCb = nullptr; /* 响应包回调 */
};
} // namespace npacket
