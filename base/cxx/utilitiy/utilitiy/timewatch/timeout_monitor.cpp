#pragma once
#include "timeout_monitor.h"

namespace utilitiy
{
TimeoutMonitor::TimeoutMonitor(const WatchFunc& captureFunc, const EndFunc& endFunc, long long timeout, const std::string& tag)
{
    m_watcher = new TimeWatcher(
        nullptr,
        [timeout, endFunc](const std::string& tag, long long elapsed) {
            if (elapsed >= timeout)
            {
                if (endFunc)
                {
                    endFunc(tag, elapsed);
                }
            }
        },
        tag);
    m_captureFunc = captureFunc;
}

TimeoutMonitor::~TimeoutMonitor()
{
    if (m_watcher)
    {
        delete m_watcher;
        m_watcher = nullptr;
    }
}

void TimeoutMonitor::capture(long long timeout, const std::string& subTag)
{
    if (m_watcher)
    {
        m_watcher->watch(subTag,
                         [timeout, captureFunc = m_captureFunc](const std::string& tag, const std::string& subTag, long long elapsed) {
                             if (elapsed >= timeout)
                             {
                                 if (captureFunc)
                                 {
                                     captureFunc(tag, subTag, elapsed);
                                 }
                             }
                         });
    }
}
} // namespace utilitiy
