#pragma once
#include <functional>
#include <mutex>

namespace algorithm
{
/**
 * @brief 原子类型(线程安全), T如果是类/结构体对象类型则需要实现运算符重载: 
 *        bool operator==(const T& other) const;
 *        bool operator>(const T& other) const;
 *        bool operator<(const T& other) const;
 */
template<typename T>
#define ATOMIC_CMP_FUNC std::function<int(const T& a, const T& b)> /* 值比较函数, 返回值: 负数-小于, 0-等于, 正数-大于 */
#define ATOMIC_EVA_CALLBACK \
    std::function<void(const T& oldVal, const T& newVal, bool changed)> /* 值赋值回调, 参数: oldVal-旧值, newVal-新值, changed-是否变化 */
class Atomic
{
public:
    Atomic() = default;

    /**
     * @brief 构造函数
     * @param value 值
     * @param cmpFunc 值比较函数, 参数: a-当前值, b-比较值, 返回: 负数-小于, 0-等于, 正数-大于
     * @param evaCb 值赋值回调, 参数: oldVal-旧值, newVal-新值, changed-是否变化
     */
    Atomic(const T& value, const ATOMIC_CMP_FUNC& cmpFunc = nullptr, const ATOMIC_EVA_CALLBACK& evaCb = nullptr)
        : m_value(value), m_cmpFunc(cmpFunc), m_evaCallback(evaCb)
    {
    }

    /**
     * @brief 设置值比较函数
     * @param cmpFunc 值比较函数, 参数: a-当前值, b-比较值, 返回: 负数-小于, 0-等于, 正数-大于
     */
    void setCmpFunc(const ATOMIC_CMP_FUNC& cmpFunc)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_cmpFunc = cmpFunc;
    }

    /**
     * @brief 设置值赋值回调
     * @param callback 值赋值回调, 参数: oldVal-旧值, newVal-新值, changed-是否变化
     */
    void setEvaCallback(const ATOMIC_EVA_CALLBACK& callback)
    {
        std::lock_guard<std::mutex> locker(m_mutex);
        m_evaCallback = callback;
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
     * @return true-值变化, false-值未变化
     */
    bool set(const T& value)
    {
        T oldVal;
        ATOMIC_EVA_CALLBACK evaCallback = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            oldVal = m_value;
            m_value = value;
            evaCallback = m_evaCallback;
        }
        auto changed = (value != oldVal);
        if (evaCallback)
        {
            evaCallback(oldVal, value, changed);
        }
        return changed;
    }

    Atomic& operator=(const T& value)
    {
        set(value);
        return (*this);
    }

    bool operator==(const T& value)
    {
        T tmpValue = T{};
        ATOMIC_CMP_FUNC compFunc = nullptr;
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
        ATOMIC_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (0 != compFunc(tmpValue, value));
        }
        return !(value == tmpValue);
    }

    bool operator>(const T& value)
    {
        T tmpValue = T{};
        ATOMIC_CMP_FUNC compFunc = nullptr;
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
        ATOMIC_CMP_FUNC compFunc = nullptr;
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
        ATOMIC_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) >= 0);
        }
        return !(tmpValue < value);
    }

    bool operator<=(const T& value)
    {
        T tmpValue = T{};
        ATOMIC_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::mutex> locker(m_mutex);
            tmpValue = m_value;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (compFunc(tmpValue, value) <= 0);
        }
        return !(tmpValue > value);
    }

private:
    std::mutex m_mutex;
    T m_value = T{}; /* 值 */
    ATOMIC_CMP_FUNC m_cmpFunc = nullptr; /* 值比较函数 */
    ATOMIC_EVA_CALLBACK m_evaCallback = nullptr; /* 值赋值回调 */
};
} // namespace algorithm
