#pragma once

#include "task.h"

#include <functional>

namespace threading
{
/**
 * @brief 简单的任务实现：基于函数封装
 */
class SimpleTask : public Task
{
public:
    /**
     * @brief 构造函数
     * @param name 任务名称
     * @param func 需要执行的函数
     */
    SimpleTask(const std::string& name, const std::function<void()>& func);

    virtual ~SimpleTask() = default;

    void run() override;

private:
    const std::function<void()> m_func; /* 任务执行函数 */
};
} /* namespace threading */
