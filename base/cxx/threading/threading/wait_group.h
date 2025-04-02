#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace threading
{
/**
 * @brief 类似Go的WaitGroup，提供线程同步机制
 */
class WaitGroup
{
public:
    WaitGroup() = default;

    /**
     * @brief 添加计数
     * @param count 计算
     */
    void add(size_t count = 1)
    {
        m_counter += count;
    }

    /**
     * @brief 消费计算
     */
    void done()
    {
        --m_counter;
        if (m_counter.load() <= 0)
        {
            m_cv.notify_all();
        }
    }

    /**
     * @brief 获取当前计算
     * @return 当前计算
     */
    int getCount()
    {
        return m_counter.load();
    }

    /**
     * @brief 等待
     */
    void wait()
    {
        if (m_counter.load() > 0)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]() { return m_counter.load() <= 0; });
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv; /* 条件变量 */
    std::atomic_size_t m_counter = {0}; /* 计数器 */
};
} // namespace threading
