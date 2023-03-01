#pragma once
#include <condition_variable>
#include <functional>
#include <initializer_list>
#include <list>
#include <mutex>

namespace threading
{
/**
 * @brief 线程安全队列, 注意: T如果是类/结构体对象, 则需要实现运算符重载: bool operator==(const T& other) const
 */
template<typename T>
class SafeQueue
{
public:
    SafeQueue() = default;
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
     * @brief 设置比较函数
     * @param cmpFunc 比较函数, 参数: a-当前值, b-比较值, 返回: 负数-小于, 0-等于, 正数-大于
     */
    void setCmpFunc(const std::function<int(const T& a, const T& b)>& cmpFunc)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_cmpFunc = cmpFunc;
    }

    /**
      * @brief 入队列
      * @param value 值
      * @param prevMode 入队列前操作模式(选填): 0.无, 1.删除首个元素, 2.清空所有元素, 3.删除相同的旧元素
      */
    void push(const T& value, int prevMode = 0)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        if (!m_list.empty())
        {
            if (1 == prevMode)
            {
                m_list.pop_front();
            }
            else if (2 == prevMode)
            {
                m_list.clear();
            }
            else if (3 == prevMode)
            {
                for (auto iter = m_list.begin(); m_list.end() != iter;)
                {
                    bool equalFlag = false;
                    if (m_cmpFunc)
                    {
                        if (0 == m_cmpFunc(value, *iter))
                        {
                            equalFlag = true;
                        }
                    }
                    else
                    {
                        equalFlag = (value == *iter);
                    }
                    if (equalFlag)
                    {
                        m_list.erase(iter++);
                    }
                    else
                    {
                        iter++;
                    }
                }
            }
        }
        m_list.emplace_back(std::move(value));
        m_cv.notify_all();
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
      * @brief 判断队列是否为空
      * @return true-空, false-非空
      */
    bool empty()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_list.empty();
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

private:
    std::mutex m_mutex;
    std::list<T> m_list; /* 数据列表 */
    std::condition_variable m_cv; /* 条件变量 */
    std::function<int(const T& a, const T& b)> m_cmpFunc = nullptr; /* 值比较函数 */
};
} // namespace threading
