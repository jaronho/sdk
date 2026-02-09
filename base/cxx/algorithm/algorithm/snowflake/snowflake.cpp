#include "snowflake.h"

#include <chrono>
#include <memory>
#include <random>
#include <thread>

namespace algorithm
{
Snowflake::Snowflake(uint64_t datacenterId, uint64_t workerId)
    : m_datacenterId(datacenterId), m_workerId(workerId), m_sequence(0), m_lock(ATOMIC_FLAG_INIT)
{
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    m_timestamp = (ntp.time_since_epoch().count() & 0x1FFFFFFFFFF);
}

uint64_t Snowflake::generate()
{
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    uint64_t newTime = (ntp.time_since_epoch().count() & 0x1FFFFFFFFFF);
    while (m_lock.test_and_set(std::memory_order_acquire)) /* 等待原子锁 */
        ;
    if (newTime == m_timestamp)
    {
        m_sequence = (m_sequence + 1) & 0xFFF;
        if (0 == m_sequence) /* 序列号溢出, 等待下一毫秒 */
        {
            while (newTime == m_timestamp)
            {
                ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
                newTime = (ntp.time_since_epoch().count() & 0x1FFFFFFFFFF);
            }
            m_timestamp = newTime;
        }
    }
    else
    {
        m_sequence = 0;
        m_timestamp = newTime;
    }
    uint64_t result = (m_timestamp << 22) | (m_datacenterId << 17) | (m_workerId << 12) | m_sequence;
    m_lock.clear(std::memory_order_release);
    return result;
}

uint64_t Snowflake::easyGenerate()
{
    static std::uniform_int_distribution<std::mt19937::result_type> s_dist(1, 99999);
    static std::mt19937 s_rng;
    static thread_local uint64_t s_workerId = 0;
    static thread_local std::unique_ptr<Snowflake> s_sf = nullptr;
    if (0 == s_workerId) /* 基于线程ID生成workerId(5位, 0-31) */
    {
        std::hash<std::thread::id> hasher;
        s_workerId = (hasher(std::this_thread::get_id()) & 0x1F);
    }
    if (!s_sf) /* 每个线程创建自己的实例, 数据中心ID=随机, 工作机器ID=线程ID哈希 */
    {
        s_rng.seed(std::random_device()());
        s_sf = std::make_unique<Snowflake>(s_dist(s_rng), s_workerId);
    }
    return s_sf->generate();
}
} // namespace algorithm
