#pragma once
#include <boost/signals2.hpp>
#include <functional>
#include <vector>

#include "signal_connection.h"

namespace threading
{
template<typename Signature>
class BasicSignal;

/**
 * @brief 简单的同步顺序调用信号(无返回值)
 */
template<typename... Args>
class BasicSignal<void(Args...)>
{
public:
    /**
     * @brief 把回调函数连接到信号
     *        注意：信号连接上之后需要通过`SignalConnection`或`ScopedSignalConnection`管理连接。
     *              在必要时主动断开, 避免函数或对象释放后回调导致内存非法访问进而崩溃。
     * @param slotFunc 槽函数
     * @return 信号连接 
     */
    SignalConnection connect(const std::function<void(Args...)>& slotFunc)
    {
        return m_signal.connect(slotFunc);
    }

    /**
     * @brief 触发信号
     * @param args 信号参数
     */
    void operator()(Args... args) const
    {
        m_signal(args...);
    }

protected:
    boost::signals2::signal<void(Args...)> m_signal;
};

/**
 * @brief 简单的同步顺序调用信号(有返回值)
 */
template<typename R, typename... Args>
class BasicSignal<R(Args...)>
{
    using Results = std::vector<R>;

public:
    /**
     * @brief 把回调函数连接到信号
     * @param slotFunc 槽函数
     * @return 信号连接 
     */
    SignalConnection connect(const std::function<R(Args...)>& slotFunc)
    {
        return m_signal.connect(slotFunc);
    }

    /**
     * @brief 触发信号
     * @param args 信号参数
     * @return 信号返回值列表
     */
    Results operator()(Args... args) const
    {
        return m_signal(args...);
    }

protected:
    /* 合并器: 把信号返回值组合成列表 */
    class Combiner
    {
    public:
        using result_type = Results;

        template<typename InputIterator>
        result_type operator()(InputIterator begin, InputIterator end) const
        {
            return result_type(begin, end);
        }
    };

    boost::signals2::signal<R(Args...), Combiner> m_signal;
};
} // namespace threading
