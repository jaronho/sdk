#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <mutex>
#include <unordered_map>

#include "../task/executor.h"

namespace threading
{
/**
 * @brief asio线程, 注意: 如果线程内执行死循环, 当要杀死线程时需要先退出循环, 否则会阻塞调用线程
 */
class AsioExecutor final : public Executor
{
public:
    /**
     * @brief 构造函数
     * @param name 线程名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param threadCount 线程个数
     */
    AsioExecutor(const std::string& name, size_t threadCount = 1);

    virtual ~AsioExecutor();

    /**
     * @brief 获取正在执行的线程数
     * @return 正在执行的线程数
     */
    size_t getBusyCount() override;

    /**
     * @brief 等待退出
     */
    void join() override;

    /**
     * @brief 把异步任务加入当前队列
     * @param task 异步任务
     * @param wait 队列满时是否等待, true-等待, false-丢弃
     * @return 异步任务(和入参一致)
     */
    TaskPtr post(const TaskPtr& task, bool wait = true) override;

    /**
     * @brief 扩展线程池
     * @param count 线程数量
     * @return 当前线程数
     */
    size_t extend(size_t count) override;

    /**
     * @brief 获取IO上下文
     * @return IO上下文
     */
    boost::asio::io_context* getContext();

private:
    void threadFunc(const std::string& name, size_t totalCount);

private:
    std::atomic_int m_threadIndex = {0}; /* 线程索引 */
    boost::asio::detail::thread_group m_threads; /* 线程组 */
    boost::asio::io_context m_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_worker;
    std::mutex m_mapMutex; /* 映射表互斥锁 */
    std::unordered_map<int, std::string> m_threadIdNameMap; /* 线程id和名称映射表 */
    std::atomic_int m_busyCount = {0}; /* 正在执行的线程数 */
};
} // namespace threading
