#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <initializer_list>
#include <list>
#include <mutex>

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
    SafeQueue& operator=(const SafeQueue&) = delete;

    /**
      * @brief 使用迭代器为参数的构造函数, 适用所有容器对象
      * @param first
      * @param last
      */
    template<typename _InputIterator>
    SafeQueue(_InputIterator first, _InputIterator last)
    {
        for (auto iter = first; last != iter; ++iter)
        {
            m_list.emplace_back(*iter);
        }
    }

    /**
      * @brief 使用初始化列表为参数的构造函数
      * @param list
      */
    SafeQueue(std::initializer_list<std::list<T>> list) : SafeQueue(list.begin, list.end) {}

    /**
      * @brief 使用队列个数上限为参数的构造函数
      * @param maxCount 队列个数上限, 0-表示不限制
      */
    SafeQueue(size_t maxCount = 0) : m_maxCount(maxCount) {}

    /**
      * @brief 设置队列个数上限
      * @param maxCount 队列个数上限, 0-表示不限制
      */
    void setMaxCount(size_t maxCount)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_maxCount = maxCount;
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param wait 队列已满的情况下, 是否等待(选填): true-等待, false-丢弃
      * @return true-成功, false-失败
      */
    bool push(const T& value, bool wait = true)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_maxCount > 0 && m_list.size() >= m_maxCount)
        {
            if (wait) /* 等待 */
            {
                m_cv.wait(locker, [this]() { return m_list.size() < m_maxCount; });
            }
            else /* 丢弃 */
            {
                return false;
            }
        }
        m_list.emplace_back(std::move(value));
        m_cv.notify_all();
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
        if (m_list.empty())
        {
            return false;
        }
        value = std::move(m_list.front());
        m_list.pop_front();
        m_cv.notify_all();
        return true;
    }

    /**
      * @brief 等待出队列(阻塞)
      * @return 值
      */
    T waitPop()
    {
        std::unique_lock<std::mutex> locker(m_mutex); /* 和条件变量一同使用需要使用unique_lock */
        m_cv.wait(locker, [&]() { return !m_list.empty(); });
        auto value = std::move(m_list.front());
        m_list.pop_front();
        m_cv.notify_all();
        return value;
    }

    /**
      * @brief 尝试获取首个元素
      * @param value [输出]值
      * @return 队列元素个数
      */
    size_t tryFront(T& value)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_list.empty())
        {
            return 0;
        }
        value = m_list.front();
        return m_list.size();
    }

    /**
      * @brief 清空队列
      */
    void clear()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_list.clear();
    }

    /**
      * @brief 队列元素个数
      * @return 元素个数
      */
    size_t size()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_list.size();
    }

    /**
      * @brief 判断队列是否为空
      * @return true-空, false-非空
      */
    bool empty()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_list.empty();
    }

    /**
      * @brief 判断队列是否已满
      * @return true-已满, false-未满
      */
    bool full()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (m_maxCount > 0 && m_list.size() >= m_maxCount)
        {
            return true;
        }
        return false;
    }

private:
    std::mutex m_mutex;
    std::list<T> m_list; /* 数据列表 */
    std::condition_variable m_cv; /* 条件变量 */
    size_t m_maxCount = 0; /* 队列个数上限, 0-表示不限制 */
};
} // namespace threading
