#include "simple_data_classifier.h"

#include <algorithm>

namespace utility
{
/**
 * @brief 限制分类器配置
 * @param cfg 外部定义的配置信息
 * @return 限制后的新配置
 */
SimpleClassifierConfig limitSimpleClassifierConfig(SimpleClassifierConfig cfg)
{
    if (0 == cfg.intervalThreshold)
    {
        cfg.intervalThreshold = 10;
    }
    if (cfg.intervalSampleCount < 2)
    {
        cfg.intervalSampleCount = 2;
    }
    if (0 == cfg.idleTimeout)
    {
        cfg.idleTimeout = 100;
    }
    if (0 == cfg.maxBufferSize)
    {
        cfg.maxBufferSize = 4096;
    }
    else if (cfg.maxBufferSize > 65535)
    {
        cfg.maxBufferSize = 65535;
    }
    return cfg;
}

SimpleDataClassifier::SimpleDataClassifier(const SimpleClassifierConfig& cfg) : m_cfg(limitSimpleClassifierConfig(cfg)) {}

void SimpleDataClassifier::setDataCallback(const DATA_CALLBACK& callback)
{
    m_dataCallback = callback;
}

void SimpleDataClassifier::recvData(const std::chrono::steady_clock::time_point& ntp, const uint8_t* data, uint32_t dataLen)
{
    if (!m_dataCallback)
    {
        return;
    }
    checkTimeout(ntp); /* 超时检查, 每次调用都检查, 确保及时性 */
    checkStreamEnd(ntp); /* 流结束检查 */
    if (!data || 0 == dataLen)
    {
        m_lastRecvTime = ntp;
        return;
    }
    if (m_buffer.empty())
    {
        m_bufferStartTime = ntp;
    }
    else /* 记录时间间隔 */
    {
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastRecvTime).count();
        if (m_intervalSamples.size() >= m_cfg.intervalSampleCount)
        {
            m_intervalSamples.pop_front();
        }
        m_intervalSamples.push_back(interval);
    }
    auto remainingSpace = m_cfg.maxBufferSize - m_buffer.size();
    if (dataLen > remainingSpace) /* 空间不足时先分发当前数据 */
    {
        if (!m_buffer.empty())
        {
            auto type = detectDataType();
            if (SimpleDataType::unknown == type)
            {
                type = m_lastDataType;
            }
            dispatchData(type, false);
        }
    }
    /* 缓存数据 */
    m_buffer.insert(m_buffer.end(), data, data + dataLen);
    m_lastRecvTime = ntp;
    /* 立即进行类型判断和分发 */
    auto type = detectDataType();
    if (SimpleDataType::command == type)
    {
        if (checkStreamFlags()) /* 命令数据需要二次判断流标识 */
        {
            m_isInStreamMode = true;
            m_lastStreamDataTime = ntp;
            dispatchData(SimpleDataType::stream, false);
        }
        else
        {
            dispatchData(type, true);
        }
    }
    else if (SimpleDataType::stream == type)
    {
        m_isInStreamMode = true;
        m_lastStreamDataTime = ntp;
        dispatchData(type, false);
    }
    else /* 未知类型, 检查流标识 */
    {
        if (checkStreamFlags())
        {
            m_isInStreamMode = true;
            m_lastStreamDataTime = ntp;
            dispatchData(SimpleDataType::stream, false);
        }
        else /* 保守策略, 按命令处理并立即结束 */
        {
            dispatchData(SimpleDataType::command, true);
        }
    }
}

void SimpleDataClassifier::reset()
{
    if (m_buffer.empty())
    {
        m_lastDataType = SimpleDataType::unknown;
        m_intervalSamples.clear();
        m_isInStreamMode = false;
    }
    else
    {
        auto type = detectDataType();
        dispatchData(type, true);
    }
}

SimpleDataType SimpleDataClassifier::detectDataType() const
{
    if (m_buffer.empty())
    {
        return SimpleDataType::unknown;
    }
    if (m_intervalSamples.size() > 0)
    {
        int64_t sum = 0;
        for (auto interval : m_intervalSamples)
        {
            sum += interval;
        }
        int64_t avgInterval = sum / m_intervalSamples.size();
        if (avgInterval >= m_cfg.intervalThreshold)
        {
            return SimpleDataType::command;
        }
        else
        {
            return SimpleDataType::stream;
        }
    }
    return SimpleDataType::unknown;
}

void SimpleDataClassifier::dispatchData(SimpleDataType type, bool isEnd)
{
    if (!m_dataCallback || m_buffer.empty())
    {
        return;
    }
    if (SimpleDataType::unknown == type)
    {
        type = SimpleDataType::command;
    }
    m_dataCallback(type, m_buffer.data(), m_buffer.size(), isEnd);
    m_buffer.clear();
    if (isEnd)
    {
        m_lastDataType = SimpleDataType::unknown;
        m_isInStreamMode = false;
    }
    else
    {
        m_lastDataType = type;
    }
    m_intervalSamples.clear();
}

void SimpleDataClassifier::checkTimeout(const std::chrono::steady_clock::time_point& ntp)
{
    if (!m_buffer.empty())
    {
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_bufferStartTime).count();
        if (interval > m_cfg.idleTimeout)
        {
            auto type = detectDataType();
            if (SimpleDataType::unknown == type)
            {
                type = checkStreamFlags() ? SimpleDataType::stream : SimpleDataType::command;
            }
            dispatchData(type, true);
        }
    }
}

void SimpleDataClassifier::checkStreamEnd(const std::chrono::steady_clock::time_point& ntp)
{
    if (m_isInStreamMode)
    {
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(ntp - m_lastStreamDataTime).count();
        if (interval >= m_cfg.idleTimeout)
        {
            if (m_buffer.empty()) /* 缓冲区为空时, 直接调用回调通知流结束 */
            {
                if (m_dataCallback)
                {
                    m_dataCallback(SimpleDataType::stream, nullptr, 0, true);
                }
                m_isInStreamMode = false;
            }
            else /* 缓冲区有数据, 分发剩余数据 */
            {
                dispatchData(SimpleDataType::stream, true);
            }
        }
    }
}

bool SimpleDataClassifier::checkStreamFlags() const
{
    for (const auto& flag : m_cfg.streamFlags)
    {
        if (flag.empty() || flag.size() > m_buffer.size())
        {
            continue;
        }
        auto iter = std::search(m_buffer.begin(), m_buffer.end(), flag.begin(), flag.end());
        if (m_buffer.end() != iter)
        {
            return true;
        }
    }
    return false;
}
} // namespace utility
