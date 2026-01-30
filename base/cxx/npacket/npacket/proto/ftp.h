#pragma once
#include <functional>
#include <memory>
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
     * @brief 数据连接模式
     */
    enum class DataMode
    {
        ACTIVE = 0, /* 主动模式 */
        PASSIVE /* 被动模式 */
    };

    /**
     * @brief 控制连接信息
     */
    struct CtrlInfo
    {
        std::string clientIp; /* 客户端IP */
        uint32_t clientPort = 0; /* 客户端控制端口 */
        std::string serverIp; /* 服务端IP */
        uint32_t serverPort = 0; /* 服务端控制端口 */
        DataMode mode = DataMode::ACTIVE; /* 数据连接模式 */
    };

    /**
     * @brief 数据包标识
     */
    enum class DataFlag
    {
        READY = 0, /* 准备传输数据(此时无数据) */
        BODY, /* 数据包内容 */
        FINISH, /* 数据传输结束(此时无数据) */
        ABNORMAL /* 异常(超时)结束(此时无头部且无数据) */
    };

    /**
     * @brief 控制包回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param flag 命令/代码
     * @param arg 参数
     */
    using CTRL_PKT_CALLBACK = std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                                 const ProtocolHeader* header, const std::string& flag, const std::string& arg)>;

    /**
     * @brief 数据包回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param ctrl 控制连接信息
     * @param flag 数据包标识
     * @param data 数据内容
     * @param dataLen 数据长度
     */
    using DATA_PKT_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                           const CtrlInfo& ctrl, const DataFlag& flag, const uint8_t* data, uint32_t dataLen)>;

public:
    /**
     * @brief 构造函数
     * @param dcTimeout 数据连接超时时间(单位: 秒)
     */
    FtpParser(uint32_t dcTimeout = 15);

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
     * @param payload 传输层负载
     * @param payloadLen 传输层负载长度
     * @param consumeLen [输出]消耗的长度
     * @return 解析结果
     */
    ParseResult parse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                      const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

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
    bool parseRequest(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                      const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析响应包
     */
    bool parseResponse(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                       const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 解析数据端口
     * @param ip_port 格式如: "192,168,31,82,195,80" 得出IP为: 192.168.31.82, 端口=195*256 + 80 = 50000
     * @param ip [输出]IP
     * @param port [输出]端口
     */
    bool parseDataPort(const std::string& ip_port, std::string& ip, uint32_t& port);

    /**
     * @brief 处理数据端口
     */
    void handleDataPort(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header, const DataMode& mode,
                        const std::string& ip, uint32_t port);

    /**
     * @brief 回收数据连接
     */
    void recyleDataConnect(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 解析数据包
     */
    bool parseData(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                   const uint8_t* payload, uint32_t payloadLen);

private:
    /**
     * @brief 数据连接状态
     */
    enum class DataConnectStatus
    {
        READY = 0, /* 就绪(协商完毕, 等待建立) */
        CREATED /* 已建立 */
    };

    /**
     * @brief 数据连接信息
     */
    struct DataConnectInfo
    {
        CtrlInfo ctrl; /* 控制连接信息 */
        std::string ip; /* 主动模式下: 客户端的IP, 被动模式下: 服务端的IP */
        uint32_t port = 0; /* 主动模式下: 客户端的端口, 被动模式下: 服务端的端口 */
        DataConnectStatus status = DataConnectStatus::READY; /* 连接状态 */
        std::chrono::steady_clock::time_point tp{}; /* 更新时间点 */
    };

private:
    const uint32_t m_dataConnectTimeout; /* 数据连接超时时间(单位: 秒) */

    CTRL_PKT_CALLBACK m_requestCb = nullptr; /* 请求包回调 */
    CTRL_PKT_CALLBACK m_responseCb = nullptr; /* 响应包回调 */
    DATA_PKT_CALLBACK m_dataCb = nullptr; /* 数据包回调 */

    std::unordered_map<std::string, std::shared_ptr<DataConnectInfo>> m_dataConnectList; /* 数据连接列表, key: IP+:+端口 */
};
} // namespace npacket
