#pragma once
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace utility
{
/** 
 * @brief 数据类型
 */
enum SimpleDataType
{
    unknown = 0, /* 未知数据类型 */
    command, /* 命令数据(低速, 间断): 手动输入, 控制指令 */
    stream, /* 流数据(高速, 连续): 文件传输, 传感器数据 */
};

/** 
 * @brief 简单分类器配置
 */
struct SimpleClassifierConfig
{
    uint32_t intervalThreshold = 10; /* 关键间隔阈值(毫秒), 实际间隔>=阈值, 判定为命令(低速), 实际间隔<阈值, 判定为流(高速) */
    uint32_t intervalSampleCount = 10; /* 时间间隔采样数, 收集连续收到数据的最近N条间隔, 然后计算时取平均值 */
    uint32_t idleTimeout = 100; /* 数据接收静默超时时间(毫秒) */
    uint32_t maxBufferSize = 4096; /* 最大缓冲区长度, 缓冲区数据超过此长度时则强制回调 */
    std::vector<std::string> streamFlags; /* 流标识, 当数据判定非流数据时, 需要检测是否包含该标识, 包含则判定为流, 例如: \r, \r\n */
};

/** 
 * @brief 简单数据分类器
 */
class SimpleDataClassifier
{
public:
    /**
     * @brief 数据回调函数类型
     * @param type 数据类型
     * @param data 数据指针(可能为空)
     * @param dataLen 数据长度(可能为0)
     * @param isEnd 是否结束标记
     */
    using DATA_CALLBACK = std::function<void(SimpleDataType type, const uint8_t* data, uint32_t dataLen, bool isEnd)>;

public:
    /**
     * @brief 构造函数
     * @param cfg 分类器配置
     */
    SimpleDataClassifier(const SimpleClassifierConfig& cfg = SimpleClassifierConfig{});

    /**
     * @brief 设置数据回调函数
     * @param callback 回调函数
     */
    void setDataCallback(const DATA_CALLBACK& callback);

    /**
     * @brief 接收数据(外部循环调用)
     * @param ntp 当前时间点
     * @param data 数据(可能为空)
     * @param dataLen 数据长度(可能为0)
     */
    void recvData(const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen);

    /**
     * @brief 重置
     */
    void reset();

private:
    /**
     * @brief 检测数据类型
     * @return 数据类型
     */
    SimpleDataType detectDataType() const;

    /** 
     * @brief 分发数据
     * @param type 数据类型
     * @param isEnd 是否结束标记
     */
    void dispatchData(SimpleDataType type, bool isEnd);

    /**
     * @brief 检查并处理超时数据
     * @param ntp 当前时间点
     */
    void checkTimeout(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 检查流数据是否结束
     * @param ntp 当前时间点
     */
    void checkStreamEnd(const std::chrono::steady_clock::time_point& ntp);

    /**
     * @brief 检查是否包含流标识
     * @return true-包含流标识 false-不包含
     */
    bool checkStreamFlags() const;

private:
    SimpleClassifierConfig m_cfg; /* 配置 */
    DATA_CALLBACK m_dataCallback; /* 数据回调函数 */
    std::vector<uint8_t> m_buffer; /* 缓冲区 */
    std::chrono::steady_clock::time_point m_bufferStartTime; /* 缓冲区开始接收数据时间点 */
    std::chrono::steady_clock::time_point m_lastRecvTime; /* 上次接收数据时间点 */
    std::chrono::steady_clock::time_point m_lastStreamDataTime; /* 最后流数据接收时间点 */
    bool m_isInStreamMode = false; /* 是否处于流模式 */
    SimpleDataType m_lastDataType = SimpleDataType::unknown; /* 上次接收数据的类型 */
    std::deque<int64_t> m_intervalSamples; /* 接收数据间隔采样 */
};
} // namespace utility
