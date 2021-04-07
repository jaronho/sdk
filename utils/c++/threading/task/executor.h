#pragma once

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
     * @brief 创建执行者
     * @param name 执行者名称
     */
    Executor(const std::string& name);

    virtual ~Executor();

    /**
	 * @brief 获取名称
	 *
	 * @return 执行者名称
	 */
    std::string GetName() const;

    /**
     * @brief 等待退出
     */
    virtual void Join() = 0;

    /**
     * @brief 把任务加入当前队列
     * @param task 任务
     * 
     * @return 任务(和入参一致)
     */
    virtual TaskPtr Post(const TaskPtr& task) = 0;

    /**
     * @brief 把回调函数作为任务加入当前队列
     * @param task 需要执行的函数
     * @param name 任务名称
     * 
     * @return 基于传入函数创建的任务
     */
    virtual TaskPtr Post(const std::function<void()>& task, const std::string& name = std::string());

private:
    const std::string m_name;
};

using ExecutorPtr = std::shared_ptr<Executor>;
} /* namespace threading */
