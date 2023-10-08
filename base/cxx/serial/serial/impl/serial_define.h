#pragma once

#include <limits>
#include <string>

namespace serial
{
/**
 * @brief 超时对象(单位:毫秒), 可以通过设置Timeout::maxMS()来关闭超时
 */
struct Timeout
{
    /**
     * @brief 获取最大超时值(毫秒)
     */
    static unsigned int maxMS()
    {
        static const unsigned int MAX_MS = (std::numeric_limits<unsigned int>::max)();
        return MAX_MS;
    }

    /**
     * @brief 简单创建超时对象
     * @param timeout 超时时间(毫秒), 定义进行读/写调用后的超时时间
     * @return 超时结构体
     */
    static Timeout simpleTimeout(unsigned int timeout)
    {
        return Timeout(maxMS(), timeout, 0, timeout, 0);
    }

    Timeout(unsigned int interByte = 0, unsigned int readConstant = 0, unsigned int readMultiplier = 0, unsigned int writeConstant = 0,
            unsigned int writeMultiplier = 0)
        : interByteTimeout(interByte)
        , readTimeoutConstant(readConstant)
        , readTimeoutMultiplier(readMultiplier)
        , writeTimeoutConstant(writeConstant)
        , writeTimeoutMultiplier(writeMultiplier)
    {
    }

    bool operator==(const Timeout& other) const
    {
        return other.interByteTimeout == interByteTimeout && other.readTimeoutConstant == readTimeoutConstant
               && other.readTimeoutMultiplier == readTimeoutMultiplier && other.writeTimeoutConstant == writeTimeoutConstant
               && other.writeTimeoutMultiplier == writeTimeoutMultiplier;
    }

    bool operator!=(const Timeout& other) const
    {
        return other.interByteTimeout != interByteTimeout || other.readTimeoutConstant != readTimeoutConstant
               || other.readTimeoutMultiplier != readTimeoutMultiplier || other.writeTimeoutConstant != writeTimeoutConstant
               || other.writeTimeoutMultiplier != writeTimeoutMultiplier;
    }

    unsigned int interByteTimeout; /* 接收到的字节之间的超时时间(单位:毫秒) */
    unsigned int readTimeoutConstant; /* 进行读调用后的超时时间计算常数 */
    unsigned int readTimeoutMultiplier; /* 进行读调用后的超时时间计算乘数 */
    unsigned int writeTimeoutConstant; /* 进行写调用后的超时时间计算常数 */
    unsigned int writeTimeoutMultiplier; /* 进行写调用后的超时时间计算乘数 */
};

/**
 * @brief 数据位
 */
enum class Databits
{
    five, /* 5位 */
    six, /* 6位 */
    seven, /* 7位 */
    eight /* 8位 */
};

/**
 * @brief 校验位
 */
enum class ParityType
{
    none, /* 无校验 */
    odd, /* 奇校验 */
    even, /* 偶校验 */
    mark, /* 校验位始终为1 */
    space /* 校验位始终为0 */
};

/**
 * @brief 停止位
 */
enum class Stopbits
{
    one, /* 1位 */
    one_and_half, /* 1.5位 */
    two /* 2位 */
};

/**
 * @brief 流控
 */
enum class FlowcontrolType
{
    none, /* 无流控 */
    software, /* 软件流控 */
    hardware /* 硬件流控 */
};

/**
 * @brief 串口设备端口描述信息
 */
struct PortInfo
{
    std::string port; /* 端口号 */
    std::string hardwareId; /* 硬件ID, 例如: USB串口设备为 VID:PID */
    std::string description; /* 人类可读的描述信息 */
    std::string location; /* 本地环境属性 */
};
} // namespace serial
