#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "blockingconcurrentqueue.h"

namespace algorithm
{
/**
 * @brief 入队策略
 */
enum class SQueuePushStrategy
{
    waitting, /* 阻塞等待 */
    drop_current, /* 丢弃当前 */
    drop_oldest /* 丢弃最旧 */
};

/**
 * @brief 线程安全队列
 */
template<typename T>
class SQueue
{
public:
    SQueue(const SQueue&) = delete;
    SQueue(SQueue&&) = default;
    SQueue& operator=(const SQueue&) = delete;
    SQueue& operator=(SQueue&&) = default;

    /**
     * @brief 构造函数
     * @param capacity 队列最大容量, 0表示不限制
     * @param dropFunc 丢弃回调函数, 参数: data-丢弃的数据
     */
    explicit SQueue(size_t capacity = 0, std::function<void(const T& data)> dropFunc = nullptr)
        : m_capacity(capacity), m_dropFunc(std::move(dropFunc)), m_stopFlag(false)
    {
    }

    ~SQueue()
    {
        stop();
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param strategy 入队策略(当有限制队列最大容量时才生效)
      * @return true-成功, false-失败(值未入队列)
      */
    bool push(const T& value, const SQueuePushStrategy& strategy = SQueuePushStrategy::waitting)
    {
        return pushInner(value, strategy);
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param strategy 入队策略(当有限制队列最大容量时才生效)
      * @return true-成功, false-失败(值未入队列)
      */
    bool push(T&& value, const SQueuePushStrategy& strategy = SQueuePushStrategy::waitting)
    {
        return pushInner(std::move(value), strategy);
    }

    /**
      * @brief 尝试出队列(非阻塞)
      * @param value [输出]值
      * @return true-成功, false-失败
      */
    bool tryPop(T& value)
    {
        return m_queue.try_dequeue(value);
    }

    /**
      * @brief 等待出队列(阻塞, 带超时)
      * @param value [输出]值
      * @param timeout 超时时间(微秒), 0表示无限等待(默认)
      * @return true-成功, false-停止
      */
    bool waitPop(T& value, size_t timeout = 0)
    {
        if (0 == timeout)
        {
            m_queue.wait_dequeue(value);
            return true;
        }
        return m_queue.wait_dequeue_timed(value, timeout);
    }

    /**
     * @brief 清空队列
     * @return 实际清空的元素个数
     */
    size_t clear()
    {
        size_t count = 0;
        T data;
        while (m_queue.try_dequeue(data))
        {
            if (m_dropFunc)
            {
                try
                {
                    m_dropFunc(data);
                }
                catch (...)
                {
                }
            }
            ++count;
        }
        return count;
    }

    /**
      * @brief 停止所有等待操作(该操作不可恢复, 明确队列不再使用时才调用), 调用后所有waitPop/push等待会立即返回false
      */
    void stop()
    {
        bool expected = false;
        if (m_stopFlag.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) /* 值发生变化 */
        {
            {
                std::lock_guard<std::mutex> locker(m_mutexStopCv);
                m_stopCv.notify_all(); /* 唤醒所有等待的生产者 */
            }
            clear(); /* 清空队列并调用丢弃回调 */
        }
    }

    /**
     * @brief 检查队列是否已停止
     * @return true-已停止, false-未停止
     */
    bool isStopped() const
    {
        return m_stopFlag.load(std::memory_order_acquire);
    }

    /**
      * @brief 队列元素个数(近似值)
      * @return 元素个数
      */
    size_t size() const
    {
        return m_queue.size_approx();
    }

    /**
      * @brief 判断队列是否为空
      * @return true-空, false-非空
      */
    bool empty() const
    {
        return (0 == m_queue.size_approx());
    }

    /**
      * @brief 判断队列是否已满
      * @return true-已满, false-未满
      */
    bool full() const
    {
        return (m_capacity > 0 && m_queue.size_approx() >= m_capacity);
    }

private:
    /**
     * @brief 通用Push逻辑
     * @param value 值
     * @param strategy 入队策略
     * @return true-成功, false-失败
     */
    template<typename U>
    bool pushInner(U&& value, const SQueuePushStrategy& strategy)
    {
        if (m_stopFlag.load(std::memory_order_acquire))
        {
            return false;
        }
        switch (strategy)
        {
        case SQueuePushStrategy::waitting:
            return pushWait(std::forward<U>(value));
        case SQueuePushStrategy::drop_current:
            return pushDropCurrent(std::forward<U>(value));
        case SQueuePushStrategy::drop_oldest:
            return pushDropOldest(std::forward<U>(value));
        }
        return false;
    }

    /**
     * @brief 阻塞等待入队(可中断)
     * @param value 值
     * @return true-成功, false-失败
     */
    template<typename U>
    bool pushWait(U&& value)
    {
        if (0 == m_capacity)
        {
            return m_queue.enqueue(std::forward<U>(value));
        }
        size_t spinCount = 0;
        while (true)
        {
            if (m_stopFlag.load(std::memory_order_acquire)) /* 高频检查停止标志 */
            {
                return false;
            }
            if (m_queue.size_approx() < m_capacity) /* 快速路径检查 */
            {
                if (m_queue.enqueue(std::forward<U>(value)))
                {
                    return true;
                }
            }
            /* 自旋阶段 */
            if (spinCount < 100)
            {
                std::this_thread::yield();
                ++spinCount;
                continue;
            }
            /* 进入可中断等待阶段 */
            std::unique_lock<std::mutex> locker(m_mutexStopCv);
            if (m_stopFlag.load(std::memory_order_acquire)) /* 双重检查, 防止在lock前被设置 */
            {
                return false;
            }
            m_stopCv.wait(locker, [this] { return (m_stopFlag.load(std::memory_order_acquire) || m_queue.size_approx() < m_capacity); });
            locker.unlock();
            spinCount = 0; /* 重置自旋计数 */
        }
    }

    /**
     * @brief 丢弃当前数据
     * @param value 值
     * @return true-成功, false-失败
     */
    template<typename U>
    bool pushDropCurrent(U&& value)
    {
        if (m_capacity > 0 && m_queue.size_approx() >= m_capacity) /* 快速容量检查 */
        {
            return false;
        }
        return m_queue.enqueue(std::forward<U>(value));
    }

    /**
     * @brief 精确丢弃最旧数据
     * @param value 值
     * @return true-成功, false-失败
     */
    template<typename U>
    bool pushDropOldest(U&& value)
    {
        if (0 == m_capacity)
        {
            return m_queue.enqueue(std::forward<U>(value));
        }
        /* 关键: 用锁包裹整个"检查-丢弃-插入"序列 */
        std::lock_guard<std::mutex> locker(m_mutexDropAction);
        if (m_queue.size_approx() < m_capacity)
        {
            return m_queue.enqueue(std::forward<U>(value));
        }
        T data;
        if (m_queue.try_dequeue(data)) /* 队列已满, 必须丢弃 */
        {
            if (m_dropFunc)
            {
                try
                {
                    m_dropFunc(std::move(data));
                }
                catch (...)
                {
                }
            }
            return m_queue.enqueue(std::forward<U>(value));
        }
        return m_queue.enqueue(std::forward<U>(value)); /* 罕见: 队列为空(其他消费者已取走), 直接入队 */
    }

private:
    const size_t m_capacity; /* 队列容量, 0表示无限制 */
    moodycamel::BlockingConcurrentQueue<T> m_queue; /* 消息队列 */
    std::function<void(const T& data)> m_dropFunc = nullptr; /* 丢弃回调 */
    std::atomic<bool> m_stopFlag{false}; /* 停止标志 */

    std::mutex m_mutexStopCv; /* 停止同步 */
    std::condition_variable_any m_stopCv; /* 支持任意锁类型 */

    std::mutex m_mutexDropAction; /* DropOldest策略专用锁 */
};
} // namespace algorithm
