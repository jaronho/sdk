#pragma once

#include <boost/fiber/all.hpp>
#include <memory>
#include <thread>

#include "../task/executor.h"

namespace threading
{
class FiberExecutor : public Executor
{
public:
    FiberExecutor(const std::string& name);

    virtual ~FiberExecutor();

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

private:
    std::unique_ptr<boost::fibers::buffered_channel<TaskPtr>> m_channel; /* 任务队列 */
    std::atomic_int m_count; /* 任务队列个数 */
    std::unique_ptr<std::thread> m_thread; /* 逻辑线程(业务线程) */
};
} /* namespace threading */
