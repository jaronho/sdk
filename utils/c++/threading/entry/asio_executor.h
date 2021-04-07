#pragma once

#include <atomic>
#include <boost/asio.hpp>

#include "../task/executor.h"

namespace threading
{
class AsioExecutor : public Executor
{
public:
    AsioExecutor(const std::string& name, size_t threadCount = 1);

    virtual ~AsioExecutor();

    /**
     * @brief 等待退出
     */
    virtual void Join();

    /**
     * @brief 把异步任务加入当前队列
     * @param task 异步任务
     * 
     * @return 异步任务(和入参一致)
     */
    virtual TaskPtr Post(const TaskPtr& task);

    /**
     * @brief 停止异步任务的处理
     */
    void Stop();

private:
    std::atomic_int m_threadIndex;
    boost::asio::detail::thread_group m_threads;
    boost::asio::io_context m_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;
};
} /* namespace threading */
