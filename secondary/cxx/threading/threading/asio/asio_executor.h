#pragma once

#include "../task/executor.h"

#include <atomic>
#include <boost/asio.hpp>
#include <unordered_map>

namespace threading
{
/**
 * @brief asio线程
 */
class AsioExecutor final : public Executor
{
public:
    /**
     * @brief 构造函数
     * @param name 线程名称
     * @param threadCount 线程个数
     */
    AsioExecutor(const std::string& name, size_t threadCount = 1);

    virtual ~AsioExecutor();

    /**
     * @brief 等待退出
     */
    void join() override;

    /**
     * @brief 把异步任务加入当前队列
     * @param task 异步任务
     * @return 异步任务(和入参一致)
     */
    TaskPtr post(const TaskPtr& task) override;

    /**
     * @brief 停止异步任务的处理
     */
    void stop();

private:
    std::atomic_int m_threadIndex = 0; /* 线程索引 */
    boost::asio::detail::thread_group m_threads; /* 线程组 */
    boost::asio::io_context m_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_worker;
    std::unordered_map<int64_t, std::string> m_threadIdNameMap; /* 线程id和名称映射表 */
};
} /* namespace threading */
