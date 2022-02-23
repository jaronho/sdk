#pragma once
#include <functional>

namespace algorithm
{
/**
 * @brief 值过滤, 用于指定相同的值需要连续重复设置多少次才成功, 过滤掉中间的一些脏数据值
 */
template<typename T>
#define TVALUE_EQUAL_FUNC std::function<bool(const T& a, const T& b)> /* 值相等比较函数, 返回值: true-相等, false-不相等 */
class TValue
{
public:
    /**
     * @brief 初始化
     * @param value 初始值
     * @param okNeedCount 成功所需次数(选填), 当相同的值连续重复设置了该次数时, 才认为设置成功, 默认为0表示每次都设置成功
     * @param allowFirstSetOk 是否允许首次设置通过即使值未改变(选填)
     * @param equalFunc 值相等比较函数(选填)
     */
    void init(const T& value, int okNeedCount = 0, bool allowFirstSetOk = false, TVALUE_EQUAL_FUNC equalFunc = nullptr)
    {
        m_realValue = value;
        m_tempValue = value;
        m_okNeedCount = okNeedCount > 0 ? okNeedCount : 0;
        m_allowFirstSetOk = allowFirstSetOk;
        m_equalFunc = equalFunc;
    }

    /**
     * @brief 设置值相等比较函数
     * @param equalFunc 比较函数
     */
    void setEqualCompareFunc(TVALUE_EQUAL_FUNC equalFunc)
    {
        m_equalFunc = equalFunc;
    }

    /**
     * @brief 设置值
     * @param nowValue 当前值
     * @return true-值更新, false-值不更新
     */
    bool set(const T& nowValue)
    {
        /* step1: 比较缓存的值和当前值是否相等 */
        bool isTempEqualNow = false;
        if (m_equalFunc)
        {
            isTempEqualNow = m_equalFunc(m_tempValue, nowValue);
        }
        else
        {
            isTempEqualNow = (m_tempValue == nowValue);
        }
        /* step2: 根据比较结果计算重复次数 */
        if (isTempEqualNow)
        {
            ++m_repeatCount;
        }
        else
        {
            m_repeatCount = 0;
        }
        /* step3: 更新缓存的值 */
        m_tempValue = nowValue;
        /* step4: 判断重复次数是否超过设置的最大次数 */
        if (m_repeatCount >= m_okNeedCount)
        {
            m_repeatCount = 0;
            bool allowSet = false; /* 是否允许更新真实值 */
            if (m_isFirstSet) /* 首次设置 */
            {
                m_isFirstSet = false;
                if (m_allowFirstSetOk) /* 首次设置允许更新真实值即使值未变化 */
                {
                    allowSet = true;
                }
            }
            if (!allowSet)
            {
                /* 比较真实的值和当前值是否相等 */
                bool isRealEqualNow = false;
                if (m_equalFunc)
                {
                    isRealEqualNow = m_equalFunc(m_realValue, nowValue);
                }
                else
                {
                    isRealEqualNow = (m_realValue == nowValue);
                }
                allowSet = (!isRealEqualNow); /* 真实的值和当前值不相等, 允许更新真实的值 */
            }
            /* 更新真实的值 */
            if (allowSet)
            {
                m_realValue = nowValue;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 获取值
     * @return 值
     */
    T get() const
    {
        return m_realValue;
    }

private:
    T m_realValue; /* 真正的值 */
    T m_tempValue; /* 缓存的值 */
    int m_okNeedCount = 0; /* 成功所需要的次数 */
    int m_repeatCount = 0; /* 连续重复次数 */
    bool m_isFirstSet = true; /* 是否首次设置 */
    bool m_allowFirstSetOk = false; /* 是否允许首次设置通过即使值没有变化 */
    TVALUE_EQUAL_FUNC m_equalFunc = nullptr; /* 值相等比较函数 */
};
} // namespace algorithm
