#pragma once
#include <functional>

#include "../protocol_parser.h"

namespace npacket
{
/**
 * @brief TPKT信息(4个字节)
 */
struct TpktInfo
{
    uint8_t version = 0; /* TPKT版本号, 通常为3 */
    uint8_t reserved = 0; /* 保留 */
    uint16_t length = 0; /* 长度(TCP的payload的长度, 包含TPKT头本身) */
};

/**
 * @brief COTP的PDU类型
 */
enum CotpPduType
{
    ED = 0x01, /* 加急数据(Expedited Data) */
    EA = 0x02, /* 加急数据确认 */
    UD = 0x04, /* 用户数据 */
    RJ = 0x05, /* 拒绝 */
    AK = 0x06, /* 数据确认 */
    ER = 0x07, /* TPDU错误 */
    DR = 0x08, /* 断开请求 */
    DC = 0x0C, /* 断开确认 */
    CC = 0x0D, /* 连接确认 */
    CR = 0x0E, /* 连接请求 */
    DT = 0x0F, /* 数据传输 */
};

/**
 * @brief COTP信息(短头, 3字节, 用于DT等数据传输)
 */
struct CotpInfo3
{
    uint8_t length = 0; /* COTP头部长度(不包含length字段本身) */
    uint8_t pduType = 0; /* PDU标识类型(对应CotpPduType) */
    uint8_t tpduNumber = 0; /* TPDU编号 */
    uint8_t lastDataUnit = 0; /* 是否最后一个数据单元, 0-No, 1-Yes */
};

/**
 * @brief COTP信息(长头, 18字节, 用于CR/CC连接建立)
 */
struct CotpInfo18
{
    uint8_t length = 0; /* COTP头部长度(不包含length字段本身) */
    uint8_t pduType = 0; /* PDU标识类型(对应CotpPduType) */
    uint16_t dstRef = 0; /* 目标引用 */
    uint16_t srcRef = 0; /* 源引用 */
    uint8_t nClass = 0; /* 协议类 */
    uint8_t extendedFormats = 0; /* 是否使用拓展样式 */
    uint8_t noExplicitFlowControl = 0; /* 是否有明确的流控制 */
    uint8_t parameterCode = 0; /* 参数代码 */
    uint8_t parameterLength = 0; /* 参数长度 */
    uint8_t reserved[9] = {0}; /* 剩余数据 */
};

/**
 * @brief COTP信息(内部集合了短头和长头信息)
 */
struct CotpInfo
{
    CotpInfo() : type(UNKNOWN), sinfo() {}

    enum /* 信息类型 */
    {
        UNKNOWN, /* 未知 */
        SHORT, /* 短头 */
        LONG /* 长头 */
    } type;

    union
    {
        CotpInfo3 sinfo; /* 短头信息 */
        CotpInfo18 linfo; /* 长头信息 */
    };
};

/**
 * @brief TPKT/COTP协议解析器
 */
class TpktCotpParser : public ProtocolParser
{
public:
    /**
     * @brief 数据回调
     * @param flag 数据标志
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param tpktInfo TPKT包信息
     * @param cotpInfo COTP包信息
     * @param payload COTP负载数据
     * @param payloadLen COTP负载长度
     */
    using DATA_CALLBACK = std::function<void(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                                             const ProtocolHeader* header, const TpktInfo& tpktInfo, const CotpInfo& cotpInfo,
                                             const uint8_t* payload, uint32_t payloadLen)>;

public:
    /**
     * @brief 获取应用层协议
     * @return 协议类型(ApplicationProtocol)
     */
    uint32_t getProtocol() const noexcept override;

    /**
     * @brief 解析
     * @param flag 数据标志
     * @param num 数据序号
     * @param ntp 数据包接收时间点
     * @param totalLen 数据包总长度
     * @param header 传输层头部
     * @param payload 传输层负载
     * @param payloadLen 传输层负载长度
     * @param consumeLen [输出]消耗的长度
     * @return 解析结果
     */
    ParseResult parse(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                      const ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen, uint32_t& consumeLen) override;

    /**
     * @brief 设置数据回调
     * @param callback 回调
     */
    void setDataCallback(const DATA_CALLBACK& callback);

private:
    /**
     * @brief 解析TPKT信息
     * @param data 数据
     * @param dataLen 数据长度
     * @param info [输出]TPKT信息
     * @param tpktLen [输出]TPKT长度
     * @return true-成功, false-失败
     */
    bool parseTpktInfo(const uint8_t* data, uint32_t dataLen, TpktInfo& info, uint32_t& tpktLen);

    /**
     * @brief 解析COTP信息
     * @param data 数据
     * @param dataLen 数据长度
     * @param info [输出]CTOP信息
     * @param cotpLen [输出]COTP长度
     * @return true-成功(info3和info18只有一个有数据), false-失败
     */
    bool parseCotpInfo(const uint8_t* data, uint32_t dataLen, CotpInfo& info, uint32_t& cotpLen);

private:
    DATA_CALLBACK m_dataCallback = nullptr; /* 数据回调 */
};
} // namespace npacket
