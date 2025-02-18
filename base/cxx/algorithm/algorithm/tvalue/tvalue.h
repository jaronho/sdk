#pragma once
#include <functional>
#include <mutex>

namespace algorithm
{
/**
 * @brief 值过滤, 用于指定相同的值需要连续重复设置多少次才成功, 过滤掉中间的一些脏数据值, 线程安全,
 *        T如果是类/结构体对象类型则需要实现运算符重载: bool operator==(const T& other) const;
 */
template<typename T>
#define TVALUE_CMP_FUNC std::function<int(const T& a, const T& b)> /* 值比较函数, 返回值: 负数-小于, 0-等于, 正数-大于 */
class TValue
{
public:
    /**
     * @brief 构造函数
     * @param value 初始值
     * @param okNeedCount 成功所需次数(选填), 当相同的值连续重复设置了该次数时, 才认为设置成功, 默认为0表示每次都设置成功
     * @param cmpFunc 值比较函数(选填)
     */
    TValue(const T& value = T{}, int okNeedCount = 0, const TVALUE_CMP_FUNC& cmpFunc = nullptr)
    {
        m_realValue = value;
        m_tempValue = value;
        m_okNeedCount = okNeedCount > 0 ? okNeedCount : 0;
        m_repeatCount = 0;
        m_cmpFunc = cmpFunc;
    }

    /**
     * @brief 拷贝构造函数
     * @param other 源对象
     */
    TValue(const TValue& other)
    {
        m_realValue = other.m_realValue;
        m_tempValue = other.m_tempValue;
        m_okNeedCount = other.m_okNeedCount;
        m_repeatCount = other.m_repeatCount;
        m_cmpFunc = other.m_cmpFunc;
    }

    /**
     * @brief 重新初始化
     * @param value 初始值
     * @param okNeedCount 成功所需次数(选填), 当相同的值连续重复设置了该次数时, 才认为设置成功, 默认为0表示每次都设置成功
     * @param cmpFunc 值相等比较函数(选填)
     */
    void reinit(const T& value, int okNeedCount = 0, const TVALUE_CMP_FUNC& cmpFunc = nullptr)
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        m_realValue = value;
        m_tempValue = value;
        m_okNeedCount = okNeedCount > 0 ? okNeedCount : 0;
        m_repeatCount = 0;
        m_cmpFunc = cmpFunc;
    }

    /**
     * @brief 设置值比较函数
     * @param cmpFunc 值比较函数
     */
    void setCmpFunc(const TVALUE_CMP_FUNC& cmpFunc)
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        m_cmpFunc = cmpFunc;
    }

    /**
     * @brief 设置值
     * @param nowValue 当前值
     * @return 0-值更新, 1-值不更新(值未发生变化), 2-值不更新(未达到重复次数)
     */
    int set(const T& nowValue)
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        /* step1: 比较缓存的值和当前值是否相等 */
        bool isTempEqualNow = false;
        if (m_cmpFunc)
        {
            isTempEqualNow = (0 == m_cmpFunc(m_tempValue, nowValue));
        }
        else
        {
            isTempEqualNow = (nowValue == m_tempValue);
        }
        /* step2: 根据比较结果计算重复次数 */
        if (isTempEqualNow)
        {
            ++m_repeatCount;
        }
        else
        {
            m_repeatCount = 1;
        }
        /* step3: 更新缓存的值 */
        m_tempValue = nowValue;
        /* step4: 判断重复次数是否超过设置的最大次数 */
        if (m_repeatCount >= m_okNeedCount)
        {
            /* 比较真实的值和当前值是否相等 */
            bool isRealEqualNow = false;
            if (m_cmpFunc)
            {
                isRealEqualNow = (0 == m_cmpFunc(m_realValue, nowValue));
            }
            else
            {
                isRealEqualNow = (nowValue == m_realValue);
            }
            if (!isRealEqualNow) /* 真实的值和当前值不相等, 更新真实的值 */
            {
                m_repeatCount = 1;
                m_realValue = nowValue;
                return 0;
            }
            else /* 真实的值和当前值相等, 不更新真实值, 重复次数减1 */
            {
                if (m_repeatCount > 1)
                {
                    --m_repeatCount;
                }
                return 1;
            }
        }
        return 2;
    }

    /**
     * @brief 获取值
     * @return 值
     */
    T get()
    {
        std::lock_guard<std::recursive_mutex> locker(m_mutex);
        return m_realValue;
    }

    TValue& operator=(const T& value)
    {
        set(value);
        return (*this);
    }

    bool operator==(const T& value)
    {
        T tmpValue = T{};
        TVALUE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            tmpValue = m_realValue;
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
        TVALUE_CMP_FUNC compFunc = nullptr;
        {
            std::lock_guard<std::recursive_mutex> locker(m_mutex);
            tmpValue = m_realValue;
            compFunc = m_cmpFunc;
        }
        if (compFunc)
        {
            return (0 != compFunc(tmpValue, value));
        }
        return !(value == tmpValue);
    }

private:
    std::recursive_mutex m_mutex;
    T m_realValue = T{}; /* 真实的值 */
    T m_tempValue = T{}; /* 缓存的值 */
    int m_okNeedCount = 0; /* 成功所需要的次数 */
    int m_repeatCount = 0; /* 连续重复次数 */
    TVALUE_CMP_FUNC m_cmpFunc = nullptr; /* 值比较函数 */
};
} // namespace algorithm
