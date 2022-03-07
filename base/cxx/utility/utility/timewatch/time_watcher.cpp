#include "time_watcher.h"

namespace utility
{
TimeWatcher::TimeWatcher(const WatchFunc& watchFunc, const EndFunc& endFunc, const std::string& tag)
    : m_begin(std::chrono::steady_clock::now()), m_watch(m_begin), m_watchFunc(watchFunc), m_endFunc(endFunc), m_tag(tag)
{
}

TimeWatcher::~TimeWatcher()
{
    if (m_endFunc)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_begin);
        m_endFunc(m_tag, elapsed.count());
    }
}

void TimeWatcher::watch(const std::string& subTag, const WatchFunc& watchFunc)
{
    std::chrono::steady_clock::time_point prevWatch = m_watch;
    m_watch = std::chrono::steady_clock::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(m_watch - prevWatch);
    if (watchFunc)
    {
        watchFunc(m_tag, subTag, elapsed.count());
    }
    else if (m_watchFunc)
    {
        m_watchFunc(m_tag, subTag, elapsed.count());
    }
}

bool TimeWatcher::check(long long timeout)
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_watch);
    if (elapsed.count() >= timeout)
    {
        m_watch = now;
        return true;
    }
    return false;
}
} // namespace utility
