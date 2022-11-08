#pragma once
#include <stdint.h>

namespace algorithm
{
/**
 * @brief snowflake序列号生成器
 */
class Snowflake
{
public:
    /**
     * @brief 构造函数
     * @param datacenterId 数据中心ID(5位), 例如: 0~99999
     * @param workerId 工作机器ID(5位), 例如: 0~99999
     */
    Snowflake(uint64_t datacenterId = 0, uint64_t workerId = 0);

    /**
     * @brief 生成序列ID
     * @return 序列ID, 例如: 6814090057511600129
     */
    uint64_t generate(void);

    /**
     * @brief 生成序列ID(自动构造对象)
     * @return 序列ID, 例如: 6814090057511600129
     */
    static uint64_t easyGenerate();

private:
    uint64_t m_flag; /* 符号位(1位),为0,通常不用 */
    uint64_t m_timestamp; /* 时间戳(41位),精确到毫秒,支持2^41/365/24/60/60/1000=69.7年 */
    /* 整个分布式系统内不会产生ID碰撞(由datacenterId和workerId作区分),支持1024个进程 */
    uint64_t m_datacenterId; /* 数据中心ID(5位) */
    uint64_t m_workerId; /* 工作机器ID(5位) */
    uint64_t m_sequence; /* 序列号(12位),每毫秒从0开始自增,支持4096个编号 */
};
} // namespace algorithm
