#pragma once
#include <boost/fiber/all.hpp>
#include <memory>
#include <thread>

#include "../task/executor.h"

namespace threading
{
/**
 * @brief fiber线程
 */
class FiberExecutor final : public Executor
{
public:
    /**
     * @brief 构造函数
     * @param name 线程名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param maxFiberCount 队列中允许的fiber最大个数, 这里默认最多1024个fiber
     * @param stackSize 每个fiber的栈空间大小(字节), 这里默认统一为512Kb
     */
    FiberExecutor(const std::string& name, size_t maxFiberCount = 1024, size_t stackSize = 512 * 1024);

    virtual ~FiberExecutor();
    
    /**
     * @brief 扩展队列大小(说明: 调用该接口无用, 对于fiber暂时不支持扩展)
     * @param count 队列大小
     * @return 当前队列大小
     */
    size_t extend(size_t count) override;

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

private:
    std::unique_ptr<boost::fibers::buffered_channel<TaskPtr>> m_channel; /* fiber队列 */
    std::atomic_int m_count = {0}; /* 任务队列个数 */
    std::unique_ptr<std::thread> m_thread; /* 线程 */
};
} // namespace threading
