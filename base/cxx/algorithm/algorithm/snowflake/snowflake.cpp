#include "snowflake.h"

#include <atomic>
#include <chrono>
#include <random>

namespace algorithm
{
Snowflake::Snowflake(uint64_t datacenterId, uint64_t workerId)
    : m_flag(0), m_datacenterId(datacenterId), m_workerId(workerId), m_sequence(0)
{
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    m_timestamp = (ntp.time_since_epoch().count() & 0x1FFFFFFFFFF);
}

uint64_t Snowflake::generate(void)
{
    auto ntp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    uint64_t newTime = (ntp.time_since_epoch().count() & 0x1FFFFFFFFFF);
    if (newTime == m_timestamp)
    {
        ++m_sequence;
    }
    else
    {
        m_sequence = 0;
        m_timestamp = newTime;
    }
    return (m_timestamp << 22) + ((m_datacenterId & 0x1F) << 17) + ((m_workerId & 0x1F) << 12) + (m_sequence & 0xFFF);
}

uint64_t Snowflake::easyGenerate(void)
{
    static std::uniform_int_distribution<std::mt19937::result_type> s_dist(1, 99999);
    static std::mt19937 s_rng;
    s_rng.seed(std::random_device()());
    static Snowflake s_sf(s_dist(s_rng), s_dist(s_rng));
    static std::atomic_flag s_glock = ATOMIC_FLAG_INIT;
    uint64_t req = 0;
    while (s_glock.test_and_set()) /* 等待原子锁 */
        ;
    req = s_sf.generate();
    s_glock.clear();
    return req;
}
} // namespace algorithm
