#pragma once
#include <functional>

#include "../task/executor.h"
#include "basic_signal.h"
#include "signal_connection.h"

namespace threading
{
template<typename Signature>
class AsyncSignal;

/**
 * 异步并发调用(无返回值)
 */
template<typename... Args>
class AsyncSignal<void(Args...)>
{
public:
    /**
     * @brief 把回调函数连接到信号
     *        注意：信号连接上之后需要通过`SignalConnection`或`ScopedSignalConnection`管理连接。
     *              在必要时主动断开, 避免函数或对象释放后回调导致内存非法访问进而崩溃。
     * @param slotFunc 槽函数
     * @param executor 执行回调的executor
     * @return 信号连接
     */
    SignalConnection connect(const std::function<void(Args...)>& slotFunc, const ExecutorPtr& executor)
    {
        const std::weak_ptr<Executor>& wpExecutor = executor;
        return m_signal.connect([wpExecutor, slotFunc = std::move(slotFunc)](Args... args) {
            const auto spExecutor = wpExecutor.lock();
            if (spExecutor)
            {
                spExecutor->post("async_signal", [slotFunc, args...]() {
                    if (slotFunc)
                    {
                        slotFunc(args...);
                    }
                });
            }
        });
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
    BasicSignal<void(Args...)> m_signal;
};
} // namespace threading
