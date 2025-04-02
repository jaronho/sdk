#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "task.h"

namespace threading
{
/**
 * @brief 执行者基类
 */
class Executor
{
public:
    /**
     * @brief 构造函数
     * @param name 执行者名称(强烈建议设置唯一标识, 以方便后续诊断)
     * @param maxCount 允许同时执行的最多任务数
     */
    Executor(const std::string& name, size_t maxCount);

    virtual ~Executor() = default;

    /**
	 * @brief 获取名称
	 * @return 执行者名称
	 */
    std::string getName() const;

    /**
     * @brief 获取允许同时执行的最多任务数
     * @return 当前最多任务数
     */
    size_t getMaxCount() const;

    /**
     * @brief 获取正在执行的任务数
     * @return 正在执行的任务数
     */
    virtual size_t getBusyCount() = 0;

    /**
     * @brief 等待退出
     */
    virtual void join() = 0;

    /**
     * @brief 把任务加入当前队列
     * @param task 任务
     * @param wait 队列满时是否等待, true-等待, false-丢弃
     * @return 任务(和入参一致)
     */
    virtual TaskPtr post(const TaskPtr& task, bool wait = true) = 0;

    /**
     * @brief 把函数作为任务加入当前队列
     * @param name 任务名称
     * @param func 需要执行的函数
     * @param wait 队列满时是否等待, true-等待, false-丢弃
     * @return 基于传入函数创建的任务
     */
    virtual TaskPtr post(const std::string& name, const std::function<void()>& func, bool wait = true);

    /**
     * @brief 扩展任务数
     * @param count 数量
     * @return 当前最多任务数
     */
    virtual size_t extend(size_t count);

private:
    const std::string m_name; /* 执行者名称 */
    std::atomic_size_t m_maxCount = {0}; /* 最多任务数 */
};

using ExecutorPtr = std::shared_ptr<Executor>;
} // namespace threading
