#pragma once
#include <functional>
#include <mutex>

namespace algorithm
{
/**
 * @brief 原子类型(线程安全)
 */
template<typename T>
#define STYPE_CMP_FUNC std::function<int(const T& a, const T& b)> /* 值相等比较函数, 返回值: 负数-小于, 0-等于, 正数-大于 */
class Atomic
{
public:
    Atomic() = default;

    /**
     * @brief 构造函数
     * @param value 值
     * @param cmpFunc 比较函数, 参数: a-当前值, b-比较值, 返回: 负数-小于, 0-等于, 正数-大于
     */
    Atomic(const T& value, const STYPE_CMP_FUNC& cmpFunc = nullptr) : m_value(value), m_cmpFunc(cmpFunc) {}

    /**
     * @brief 设置值比较函数
     * @param cmpFunc 比较函数, 参数: a-当前值, b-比较值, 返回: 负数-小于, 0-等于, 正数-大于
     */
    void setCmpFunc(const STYPE_CMP_FUNC& cmpFunc)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_cmpFunc = cmpFunc;
    }

    /**
     * @brief 获取值
     * @return 值
     */
    T get()
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        return m_value;
    }

    /**
     * @brief 设置值
     * @param value 值
     */
    void set(const T& value)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_value = value;
    }

    Atomic& operator=(const T& value)
    {
        set(value);
        return (*this);
    }

    bool operator==(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (0 == compFunc(tmpValue, value));
        }
        return (value == tmpValue);
    }

    bool operator!=(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (0 != compFunc(tmpValue, value));
        }
        return (value != tmpValue);
    }

    bool operator>(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) > 0);
        }
        return (tmpValue > value);
    }

    bool operator<(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) < 0);
        }
        return (tmpValue < value);
    }

    bool operator>=(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) >= 0);
        }
        return (tmpValue >= value);
    }

    bool operator<=(const T& value)
    {
        T tmpValue = T{};
        STYPE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) <= 0);
        }
        return (tmpValue <= value);
    }

private:
    std::mutex m_mutex;
    T m_value = T{}; /* 值 */
    STYPE_CMP_FUNC m_cmpFunc = nullptr; /* 值比较函数 */
};
} // namespace algorithm
