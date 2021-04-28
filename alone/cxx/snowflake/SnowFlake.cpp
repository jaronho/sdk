/**********************************************************************
* Author:	jaron.ho
* Date:		2020-08-27
* Brief:	snow flake序列号生成器
**********************************************************************/
#include "SnowFlake.h"
#include <atomic>
#include <chrono>

SnowFlake::SnowFlake(uint64_t datacenterId = 0, uint64_t workerId = 0)
    : m_flag(0)
    , m_datacenterId(datacenterId)
    , m_workerId(workerId)
    , m_sequence(0) {
    auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    m_timestamp = (t.time_since_epoch().count() & 0x1FFFFFFFFFF);
}
    
uint64_t SnowFlake::generate(void) {
    auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    uint64_t newTime = (t.time_since_epoch().count() & 0x1FFFFFFFFFF);
    if (newTime == m_timestamp) {
        ++m_sequence;
    } else {
        m_sequence = 0;
        m_timestamp = newTime;
    }
    return (m_timestamp << 22) + ((m_datacenterId & 0x1F) << 17) + ((m_workerId & 0x1F) << 12) + (m_sequence & 0xFFF);
}

uint64_t generateSnowFlakeSeqId(void) {
    static SnowFlake s_sf;
    static std::atomic_flag s_glock = ATOMIC_FLAG_INIT;
    int64_t req;
    while (s_glock.test_and_set()) /* 等待原子锁 */
        ;
    req = s_sf.generate();
    s_glock.clear();
    return req;
}
