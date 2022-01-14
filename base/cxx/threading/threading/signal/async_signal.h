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
     * @param callback 回调函数
     * @param executor 执行回调的executor
     * @return 信号连接
     */
    SignalConnection connect(const std::function<void(Args...)>& callback, const ExecutorPtr& executor)
    {
        const std::weak_ptr<Executor>& wpExecutor = executor;
        return m_signal.connect([wpExecutor, callback = std::move(callback)](Args... args) {
            const auto spExecutor = wpExecutor.lock();
            if (spExecutor)
            {
                spExecutor->post("async_signal", [callback, args...]() {
                    if (callback)
                    {
                        callback(args...);
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
