#pragma once
#include <chrono>
#include <functional>
#include <unordered_map>

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
     * @brief 控制包回调
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param flag 命令/代码
     * @param arg 参数
     */
    using CTRL_PKT_CALLBACK = std::function<void(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const std::string& flag,
                                                 const std::string& arg)>;

    /**
     * @brief 数据包回调
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param mode 端口模式, 1-主动模式, 2-被动模式
     * @param flag 包标识, 1-准备传输数据(此时无数据), 2-数据包, 3-数据传输结束(此时无数据)
     * @param data 数据内容
     * @param dataLen 数据长度
     */
    using DATA_PKT_CALLBACK = std::function<void(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, uint32_t mode,
                                                 uint32_t flag, const uint8_t* data, uint32_t dataLen)>;

public:
    /**
     * @brief 构造函数
     * @param dcTimeout 数据通道超时时间(单位: 秒)
     */
    FtpParser(uint32_t dcTimeout = 15);

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
     * @param callback 回调
     */
    void setRequestCallback(const CTRL_PKT_CALLBACK& callback);

    /**
     * @brief 设置响应包回调
     * @param callback 回调
     */
    void setResponseCallback(const CTRL_PKT_CALLBACK& callback);

    /**
     * @brief 设置数据包回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_PKT_CALLBACK& callback);

private:
    /**
     * @brief 解析请求包
     */
    bool parseRequest(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析响应包
     */
    bool parseResponse(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析数据端口
     * @param mode 模式, 1-主动模式, 2-被动模式
     * @param ip_port 格式如: "192,168,31,82,195,80" 得出IP为: 192.168.31.82, 端口=195*256 + 80 = 50000
     */
    bool parseDataPort(int mode, const std::string& ip_port);

    /**
     * @brief 回收数据通道
     */
    void recyleDataChannel();

    /**
     * @brief 解析数据包
     */
    bool parseData(uint32_t totalLen, const std::shared_ptr<ProtocolHeader>& header, const uint8_t* payload, uint32_t payloadLen);

private:
    /**
     * @brief 数据通道信息
     */
    struct DataChannelInfo
    {
        uint32_t mode = 1; /* 端口模式: 1-主动模式, 2-被动模式 */
        std::string ip; /* 主动模式下: 客户端的IP, 被动模式下: 服务端的IP */
        uint32_t port = 0; /* 主动模式下: 客户端的端口, 被动模式下: 服务端的端口 */
        std::chrono::steady_clock::time_point tp{}; /* 数据通道更新时间点 */
        uint32_t status = 0; /* 0-协商完毕(等待建立), 1-通道建立 */
    };

private:
    CTRL_PKT_CALLBACK m_requestCb = nullptr; /* 请求包回调 */
    CTRL_PKT_CALLBACK m_responseCb = nullptr; /* 响应包回调 */
    DATA_PKT_CALLBACK m_dataCb = nullptr; /* 数据包回调 */
    std::unordered_map<std::string, std::shared_ptr<DataChannelInfo>> m_dataChannelList; /* 数据通道信息列表, key: IP+:+端口 */
    uint32_t m_dataChannelTimeout = 15; /* 数据通道超时时间(单位: 秒) */
};
} // namespace npacket
