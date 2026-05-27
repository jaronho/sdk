#pragma once
#include <functional>
#include <memory>
#include <unordered_map>

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
     * @brief 构造函数
     * @param fragTimeout 分片重组超时时间(单位:毫秒), 值: [1秒, 5分钟]
     */
    S7CommParser(size_t fragTimeout = 30000);

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

    /**
     * @brief 清理超时缓存
     * @param ntp 当前时间点
     */
    void cleanupFragmentCache(const std::chrono::steady_clock::time_point& ntp);

private:
    /**
     * @brief 解析S7COMM信息
     * @param ntp 数据包接收时间点
     */
    bool parseS7CommInfo(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header, const uint8_t* data,
                         uint32_t dataLen, s7comm::S7CommInfo& info);

    /**
     * @brief 解析S7COMM头部
     */
    bool parseS7Header(const uint8_t* data, uint32_t dataLen, s7comm::S7Header& header, uint32_t& headerLen);

    /**
     * @brief 解析CPU服务
     */
    bool parseCpuServiceParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceParam& param);
    bool parseCpuServiceBlockListData(const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceBlockListOfTypeData(bool isRequest, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceBlockInfoData(bool isRequest, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceSzlData(const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceMessageData(bool isRequest, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceTimestamp(const uint8_t* data, uint32_t dataLen, s7comm::S7Timestamp& ts);
    bool parseCpuServiceDataDetail(const s7comm::CpuServiceParam& param, s7comm::CpuServiceData& dataOut);
    bool parseCpuServiceData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut);

    /**
     * @brief 尝试重组CPU服务数据分片(USERDATA类型)
     * @param ntp 数据包接收时间点
     * @param header 传输层头部(用于提取4-tuple作为连接标识)
     * @param info S7COMM信息(输入输出)
     * @return true-已重组完成(或无需重组), false-当前分片已缓存但未完整
     */
    bool tryReassembleCpuServiceData(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header,
                                     s7comm::S7CommInfo& info);

    /**
     * @brief 解析读/写
     */
    bool parseReadWriteParamItem(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteParamItem& item, uint32_t& itemLen);
    bool parseReadWriteParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteParam& param);
    bool parseReadWriteDataItem(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteData& item, uint32_t& itemLen);
    bool parseReadWriteData(const s7comm::RosctrType& rosctr, const s7comm::FunctionCode& funcCode, const uint8_t* data, uint32_t dataLen,
                            uint8_t itemCount, std::vector<s7comm::ReadWriteData>& dataOut);

    /**
     * @brief 解析请求下载
     */
    bool parseRequestDownloadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                   s7comm::RequestDownloadParam& param);
    bool parseRequestDownloadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                  s7comm::RequestDownloadData& dataOut);

    /**
     * @brief 解析下载
     */
    bool parseDownloadBlockParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                 s7comm::DownloadBlockParam& param);
    bool parseDownloadBlockData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                s7comm::DownloadBlockData& dataOut);

    /**
     * @brief 解析下载结束
     */
    bool parseDownloadEndedParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                 s7comm::DownloadEndedParam& param);
    bool parseDownloadEndedData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                s7comm::DownloadEndedData& dataOut);

    /**
     * @brief 解析开始上传
     */
    bool parseStartUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::StartUploadParam& param);
    bool parseStartUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::StartUploadData& dataOut);

    /**
     * @brief 解析上传
     */
    bool parseUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::UploadParam& param);
    bool parseUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::UploadData& dataOut);

    /**
     * @brief 解析上传结束
     */
    bool parseEndUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::EndUploadParam& param);
    bool parseEndUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::EndUploadData& dataOut);

    /**
     * @brief 解析PLC控制
     */
    bool parsePlcControlParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcControlParam& param);
    bool parsePlcControlData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcControlData& dataOut);

    /**
     * @brief 解析PI服务
     */
    bool parsePiServiceParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PiServiceParam& param);
    bool parsePiServiceData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PiServiceData& dataOut);

    /**
     * @brief 解析PLC停止
     */
    bool parsePlcStopParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcStopParam& param);
    bool parsePlcStopData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcStopData& dataOut);

    /**
     * @brief 解析拷贝RAM到ROM
     */
    bool parseCopyRamToRomParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CopyRamToRomParam& param);
    bool parseCopyRamToRomData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CopyRamToRomData& dataOut);

    /**
     * @brief 解析压缩内存
     */
    bool parseCompressParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CompressParam& param);
    bool parseCompressData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CompressData& dataOut);

    /**
     * @brief 解析块操作
     */
    bool parseBlockOperationParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                  s7comm::BlockOperationParam& param);
    bool parseBlockOperationData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                 s7comm::BlockOperationData& dataOut);

    /**
     * @brief 解析通信协商
     */
    bool parseSetupCommParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::SetupCommParam& param);
    bool parseSetupCommData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::SetupCommData& dataOut);

private:
    /**
     * @brief CPU服务分片缓存Key(4-tuple + dataUnitReferenceNumber)
     * @note 避免多连接间dataUnitReferenceNumber冲突
     */
    struct CpuServiceFragmentKey
    {
        uint32_t srcIp = 0; /* 源IP */
        uint32_t dstIp = 0; /* 目的IP */
        uint16_t srcPort = 0; /* 源端口 */
        uint16_t dstPort = 0; /* 目的端口 */
        uint8_t dataUnitReferenceNumber = 0; /* 数据单元参考号 */

        bool operator==(const CpuServiceFragmentKey& other) const
        {
            return srcIp == other.srcIp && dstIp == other.dstIp && srcPort == other.srcPort && dstPort == other.dstPort
                   && dataUnitReferenceNumber == other.dataUnitReferenceNumber;
        }

        bool operator<(const CpuServiceFragmentKey& other) const
        {
            if (srcIp != other.srcIp)
            {
                return srcIp < other.srcIp;
            }
            if (dstIp != other.dstIp)
            {
                return dstIp < other.dstIp;
            }
            if (srcPort != other.srcPort)
            {
                return srcPort < other.srcPort;
            }
            if (dstPort != other.dstPort)
            {
                return dstPort < other.dstPort;
            }
            return dataUnitReferenceNumber < other.dataUnitReferenceNumber;
        }
    };

    struct CpuServiceFragmentKeyHash
    {
        size_t operator()(const CpuServiceFragmentKey& key) const noexcept
        {
            size_t h = key.srcIp;
            h = h * 31 + key.dstIp;
            h = h * 31 + key.srcPort;
            h = h * 31 + key.dstPort;
            h = h * 31 + key.dataUnitReferenceNumber;
            return h;
        }
    };

    /**
     * @brief S7COMM USERDATA分片缓存
     */
    struct S7FragmentCache
    {
        std::chrono::steady_clock::time_point lastAccess; /* 最近访问时间点 */
        uint16_t firstProtocolDataUnitReference = 0; /* 第一片的PDU参考号(用于重组后更新header) */
        uint16_t totalDataLength = 0; /* 累计的S7头部dataLength(用于重组后更新header) */
        std::vector<uint8_t> data; /* 累计的数据域内容(不含returnCode/transportSize/length头部) */
    };

private:
    const size_t m_fragTimeout; /* 分片重组超时时间(单位:毫秒) */
    FRAME_CALLBACK m_frameCb = nullptr; /* 帧回调 */
    std::unordered_map<CpuServiceFragmentKey, S7FragmentCache, CpuServiceFragmentKeyHash> m_cpuServiceFragments; /* CPU服务分片缓存 */
};
} // namespace npacket
