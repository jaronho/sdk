#pragma once
#include <functional>
#include <memory>

#include "s7comm_def.h"

namespace npacket
{
/**
 * ┌─────────────────────────────────────┐
 * │  S7COMM(应用层)                     │  <- 实际的S7功能指令(读/写数据)
 * │  - 功能码, 参数, 数据                │
 * ├─────────────────────────────────────┤
 * │  COTP(会话/传输层)                   │  <- 连接管理, 可靠传输
 * │  - PDU类型, TPDU编号                 │
 * ├─────────────────────────────────────┤
 * │  TPKT(传输层适配)                    │  <- TCP上的ISO传输封装
 * │  - 版本、长度                        │
 * ├─────────────────────────────────────┤
 * │  TCP(传输层)                        │
 * ├─────────────────────────────────────┤
 * │  IP(网络层)                         │
 * ├─────────────────────────────────────┤
 * │  以太网(数据链路/物理层)             │
 * └─────────────────────────────────────┘
 */
/**
 * @brief S7COMM协议解析器
 */
class S7CommParser
{
public:
    /**
     * @brief 帧回调
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部(TCP)
     * @param tpktInfo TPKT包信息
     * @param cotpInfo COTP包信息
     * @param s7commInfo S7COMM信息
     */
    using FRAME_CALLBACK =
        std::function<void(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
                           const TpktInfo& tpktInfo, const CotpInfo& cotpInfo, const s7comm::S7CommInfo& s7commInfo)>;

public:
    /**
     * @brief 解析
     * @param flag 数据标志
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param tpktInfo TPKT包信息
     * @param cotpInfo COTP包信息
     * @param payload COTP负载数据
     * @param payloadLen COTP负载长度
     * @return 解析结果, true-成功, false-失败
     */
    bool parse(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const ProtocolHeader* header,
               const TpktInfo& tpktInfo, const CotpInfo& cotpInfo, const uint8_t* payload, uint32_t payloadLen);

    /**
     * @brief 设置帧回调
     * @param callback 回调
     */
    void setFrameCallback(const FRAME_CALLBACK& callback);

private:
    /**
     * @brief 解析S7COMM信息
     */
    bool parseS7CommInfo(const uint8_t* data, uint32_t dataLen, s7comm::S7CommInfo& info);

    /**
     * @brief 解析S7COMM头部
     */
    bool parseS7Header(const uint8_t* data, uint32_t dataLen, s7comm::S7Header& header, uint32_t& headerLen);

    /**
     * @brief 解析单个变量条目
     */
    bool parseItemSpec(const uint8_t* data, uint32_t dataLen, s7comm::ItemSpec& item, uint32_t& itemLen);

    /**
     * @brief 解析读/写参数
     */
    bool parseReadWriteParam(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteParam& param);

    /**
     * @brief 解析请求下载参数
     */
    bool parseRequestDownloadParam(const uint8_t* data, uint32_t dataLen, s7comm::RequestDownloadParam& param);

    /**
     * @brief 解析开始上传参数
     */
    bool parseStartUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::StartUploadParam& param);

    /**
     * @brief 解析上传参数
     */
    bool parseUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::UploadParam& param);

    /**
     * @brief 解析结束上传参数
     */
    bool parseEndUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::EndUploadParam& param);

    /**
     * @brief 解析PLC控制参数
     */
    bool parsePlcControlParam(const uint8_t* data, uint32_t dataLen, s7comm::PlcControlParam& param);

    /**
     * @brief 解析PI服务参数
     */
    bool parsePiServiceParam(const uint8_t* data, uint32_t dataLen, s7comm::PiServiceParam& param);

    /**
     * @brief 解析PLC停止参数
     */
    bool parsePlcStopParam(const uint8_t* data, uint32_t dataLen, s7comm::PlcStopParam& param);

    /**
     * @brief 解析块操作参数
     */
    bool parseBlockOperationParam(const uint8_t* data, uint32_t dataLen, s7comm::BlockOperationParam& param);

    /**
     * @brief 解析通信协商参数
     */
    bool parseSetupCommParam(const uint8_t* data, uint32_t dataLen, s7comm::SetupCommParam& param);

    /**
     * @brief 解析单个数据项
     */
    bool parseDataItem(const uint8_t* data, uint32_t dataLen, s7comm::DataItem& item, uint32_t& itemLen);

    /**
     * @brief 解析数据项
     */
    bool parseDataItems(const uint8_t* data, uint32_t dataLen, uint8_t itemCount, std::vector<s7comm::DataItem>& items);

private:
    FRAME_CALLBACK m_frameCb = nullptr; /* 帧回调 */
};
} // namespace npacket
