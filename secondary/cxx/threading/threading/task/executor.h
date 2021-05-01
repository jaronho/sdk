#pragma once

#include "task.h"

#include <functional>
#include <memory>
#include <string>

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
     * @param name 执行者名称
     */
    Executor(const std::string& name);

    virtual ~Executor() = default;

    /**
	 * @brief 获取名称
	 * @return 执行者名称
	 */
    std::string getName() const;

    /**
     * @brief 等待退出
     */
    virtual void join() = 0;

    /**
     * @brief 把任务加入当前队列
     * @param task 任务
     * @return 任务(和入参一致)
     */
    virtual TaskPtr post(const TaskPtr& task) = 0;

    /**
     * @brief 把函数作为任务加入当前队列
     * @param name 任务名称
     * @param func 需要执行的函数
     * @return 基于传入函数创建的任务
     */
    virtual TaskPtr post(const std::string& name, const std::function<void()>& func);

private:
    const std::string m_name; /* 执行者名称 */
};

using ExecutorPtr = std::shared_ptr<Executor>;
} // namespace threading
