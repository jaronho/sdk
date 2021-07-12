#pragma once
#include <functional>

namespace algorithm
{
/**
 * @brief 值过滤, 用于指定相同的值需要连续重复设置多少次才成功, 过滤掉中间的一些脏数据值
 */
template<typename T>
#define VALUE_FILTER_EQUAL_COMPARE_FUNC std::function<bool(T a, T b)> /* 值相等比较函数, 相等返回true, 否则返回false */
class ValueFilter
{
public:
    ValueFilter(void)
    {
        m_okNeedCount = 0;
        m_repeatCount = 0;
        m_equalCompareFunc = nullptr;
    }

    /**
     * @brief 初始化
     * @param value 初始值
     * @param okNeedCount 成功所需次数, 当相同的值连续重复设置了该次数时, 才认为设置成功, 默认为0表示每次都设置成功
     * @param equalCompareFunc 比较函数
     */
    void init(T value, int okNeedCount = 0, VALUE_FILTER_EQUAL_COMPARE_FUNC equalCompareFunc = nullptr)
    {
        m_realValue = value;
        m_tempValue = value;
        m_okNeedCount = okNeedCount > 0 ? okNeedCount : 0;
        m_repeatCount = 0;
        m_equalCompareFunc = equalCompareFunc;
    }

    /**
     * @brief 设置值相等比较函数
     * @param equalCompareFunc 比较函数
     */
    void setEqualCompareFunc(VALUE_FILTER_EQUAL_COMPARE_FUNC equalCompareFunc)
    {
        m_equalCompareFunc = equalCompareFunc;
    }

    /**
     * @brief 设置值
     * @param nowValue 当前值
     * @return true-设置成功, false-设置失败
     */
    bool set(T nowValue)
    {
        /* step1: 比较缓存的值和当前值是否相等 */
        bool isTempEqualNow = false;
        if (m_equalCompareFunc)
        {
            isTempEqualNow = m_equalCompareFunc(m_tempValue, nowValue);
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
            /* step4-1: 比较真实的值和当前值是否相等 */
            bool isRealEqualNow = false;
            if (m_equalCompareFunc)
            {
                isRealEqualNow = m_equalCompareFunc(m_realValue, nowValue);
            }
            else
            {
                isRealEqualNow = (m_realValue == nowValue);
            }
            /* step4-2: 真实的值和当前值不相等, 更新真实的值 */
            if (!isRealEqualNow)
            {
                m_realValue = nowValue;
            }
            return true;
        }
        return false;
    }

    /**
     * @brief 获取值
     * @return 值
     */
    T get(void)
    {
        return m_realValue;
    }

private:
    T m_realValue; /* 真正的值 */
    T m_tempValue; /* 缓存的值 */
    int m_okNeedCount; /* 成功所需要的次数 */
    int m_repeatCount; /* 连续重复次数 */
    VALUE_FILTER_EQUAL_COMPARE_FUNC m_equalCompareFunc; /* 值比较函数 */
};
} // namespace algorithm
