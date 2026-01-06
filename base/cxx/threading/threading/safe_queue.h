#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <initializer_list>
#include <mutex>
#include <optional>

namespace threading
{
/**
 * @brief 线程安全队列, T如果是类/结构体对象类型则需要实现运算符重载: bool operator==(const T& other) const;
 */
template<typename T>
class SafeQueue
{
public:
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue(SafeQueue&&) = default;
    SafeQueue& operator=(const SafeQueue&) = delete;
    SafeQueue& operator=(SafeQueue&&) = default;

    /**
      * @brief 使用迭代器为参数的构造函数, 适用所有容器对象
      * @param first
      * @param last
      */
    template<typename InputIterator>
    SafeQueue(InputIterator first, InputIterator last)
    {
        for (auto iter = first; last != iter; ++iter)
        {
            m_queue.emplace_back(*iter);
        }
    }

    /**
      * @brief 使用初始化列表为参数的构造函数
      * @param list
      */
    SafeQueue(std::initializer_list<T> list) : SafeQueue(list.begin(), list.end()) {}

    /**
      * @brief 使用队列个数上限为参数的构造函数
      * @param maxCount 队列个数上限, 0-表示不限制(默认)
      */
    SafeQueue(size_t maxCount = 0) : m_maxCount(maxCount) {}

    /**
      * @brief 析构函数
      */
    ~SafeQueue()
    {
        stop();
    }

    /**
      * @brief 设置队列个数上限
      * @param maxCount 队列个数上限, 0-表示不限制
      */
    void setMaxCount(size_t maxCount)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped || maxCount == m_maxCount)
        {
            return;
        }
        m_maxCount = maxCount;
        if (maxCount > 0 && m_queue.size() > maxCount) /* 缩小容量时丢弃多余元素 */
        {
            m_queue.resize(maxCount);
        }
        m_cvNotFull.notify_all(); /* 只通知受影响的等待方 */
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param flag 队列已满的情况下, 操作模式(选填): 0-等待(默认), 1-丢弃最早, 2-丢弃当前
      * @return true-成功, false-失败(值未入队列)
      */
    bool push(const T& value, int flag = 0)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped)
        {
            return false;
        }
        if (m_maxCount > 0 && m_queue.size() >= m_maxCount)
        {
            if (0 == flag) /* 等待 */
            {
                m_cvNotFull.wait(locker, [this]() { return (m_stopped || 0 == m_maxCount || m_queue.size() < m_maxCount); });
                if (m_stopped)
                {
                    return false;
                }
            }
            else if (1 == flag) /* 丢弃最早 */
            {
                m_queue.pop_front();
                m_cvNotFull.notify_one(); /* 丢弃后队列不满, 通知一个等待的生产者 */
            }
            else /* (2 == flag)丢弃当前 */
            {
                return false;
            }
        }
        m_queue.emplace_back(value);
        m_cvNotEmpty.notify_one(); /* 成功入队, 通知一个等待的消费者 */
        return true;
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param flag 队列已满的情况下, 操作模式(选填): 0-等待(默认), 1-丢弃最早, 2-丢弃当前
      * @return true-成功, false-失败(值未入队列)
      */
    bool push(T&& value, int flag = 0)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped)
        {
            return false;
        }
        if (m_maxCount > 0 && m_queue.size() >= m_maxCount)
        {
            if (0 == flag) /* 等待 */
            {
                m_cvNotFull.wait(locker, [this]() { return (m_stopped || 0 == m_maxCount || m_queue.size() < m_maxCount); });
                if (m_stopped)
                {
                    return false;
                }
            }
            else if (1 == flag) /* 丢弃最早 */
            {
                m_queue.pop_front();
                m_cvNotFull.notify_one(); /* 丢弃后队列不满, 通知一个等待的生产者 */
            }
            else /* (2 == flag)丢弃当前 */
            {
                return false;
            }
        }
        m_queue.emplace_back(std::move(value));
        m_cvNotEmpty.notify_one(); /* 成功入队, 通知一个等待的消费者 */
        return true;
    }

    /**
      * @brief 尝试出队列
      * @param value [输出]值
      * @return true-成功, false-失败
      */
    bool tryPop(T& value)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped || m_queue.empty())
        {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop_front();
        m_cvNotFull.notify_one(); /* 成功出队, 通知一个等待的生产者 */
        return true;
    }

    /**
      * @brief 等待出队列(阻塞)
      * @param value [输出]值
      * @return true-成功, false-停止
      */
    bool waitPop(T& value)
    {
        std::unique_lock<std::mutex> locker(m_mutex); /* 和条件变量一同使用需要使用unique_lock */
        m_cvNotEmpty.wait(locker, [&]() { return m_stopped || !m_queue.empty(); });
        if (m_stopped)
        {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop_front();
        m_cvNotFull.notify_one(); /* 成功出队, 通知一个等待的生产者 */
        return true;
    }

    /**
      * @brief 尝试获取首个元素(不会实际从队列中删除元素)
      * @param value [输出]值
      * @return 返回当前队列元素个数
      */
    size_t tryFront(T& value)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped || m_queue.empty())
        {
            return 0;
        }
        value = m_queue.front(); /* 这里拷贝, 不使用移动 */
        return m_queue.size();
    }

    /**
      * @brief 删除首个匹配的元素
      * @param value 要删除的值
      * @param cmpFunc 元素比较函数(选填), 参数: a-值1, b-值2, 返回值: true-值相等, false-值不相等
      * @return true-成功删除, false-未找到该元素
      */
    bool remove(const T& value, const std::function<bool(const T& a, const T& b)>& cmpFunc = nullptr)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped)
        {
            return false;
        }
        for (auto iter = m_queue.begin(); iter != m_queue.end(); ++iter)
        {
            bool match = cmpFunc ? cmpFunc(*iter, value) : (*iter == value);
            if (match)
            {
                iter = m_queue.erase(iter);
                m_cvNotFull.notify_one(); /* 删除元素后队列不满, 通知生产者 */
                return true;
            }
        }
        return false;
    }

    /**
      * @brief 删除所有匹配的元素
      * @param value 要删除的值
      * @param cmpFunc 元素比较函数(选填), 参数: a-值1, b-值2, 返回值: true-值相等, false-值不相等
      * @return 删除的元素个数
      */
    size_t removeAll(const T& value, const std::function<bool(const T& a, const T& b)>& cmpFunc = nullptr)
    {
        size_t count = 0;
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped)
        {
            return 0;
        }
        auto iter = m_queue.begin();
        while (m_queue.end() != iter)
        {
            bool match = cmpFunc ? cmpFunc(*iter, value) : (*iter == value);
            if (match)
            {
                iter = m_queue.erase(iter);
                ++count;
            }
            else
            {
                ++iter;
            }
        }
        if (count > 0)
        {
            m_cvNotFull.notify_all(); /* 删除可能腾出多个位置, 通知所有等待的生产者 */
        }
        return count;
    }

    /**
      * @brief 清空队列
      */
    void clear()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_stopped)
        {
            return;
        }
        m_queue.clear();
        m_cvNotFull.notify_all(); /* 空后队列不满, 通知所有等待的生产者 */
    }

    /**
      * @brief 队列元素个数
      * @return 元素个数
      */
    size_t size() const
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_queue.size();
    }

    /**
      * @brief 判断队列是否为空
      * @return true-空, false-非空
      */
    bool empty() const
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_queue.empty();
    }

    /**
      * @brief 判断队列是否已满
      * @return true-已满, false-未满
      */
    bool full() const
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return (m_maxCount > 0 && m_queue.size() >= m_maxCount);
    }

    /**
      * @brief 停止所有等待操作(该操作不可恢复, 明确队列不再使用时才调用), 调用后所有waitPop/push等待会立即返回false
      */
    void stop()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (!m_stopped)
        {
            m_stopped = true;
            m_cvNotEmpty.notify_all(); /* 通知所有消费者 */
            m_cvNotFull.notify_all(); /* 通知所有生产者 */
        }
    }

    /**
      * @brief 检查是否已停止
      * @return true-已停止, false-未停止
      */
    bool isStopped() const
    {
        return m_stopped.load(std::memory_order_relaxed);
    }

private:
    mutable std::mutex m_mutex;
    std::deque<T> m_queue; /* 数据列表 */
    std::condition_variable m_cvNotFull; /* 生产者使用, 队列不满条件变量 */
    std::condition_variable m_cvNotEmpty; /* 消费者使用, 队列不空条件变量 */
    size_t m_maxCount = 0; /* 队列个数上限, 0-表示不限制 */
    std::atomic_bool m_stopped{false}; /* 停止标志 */
};
} // namespace threading
