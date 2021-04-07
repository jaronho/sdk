#pragma once

#include <functional>

#include "task.h"

namespace threading
{
/**
 * @brief 简单的异步任务实现：基于回调函数封装
 */
class SimpleTask : public Task
{
public:
    using CallbackType = std::function<void()>;

    /**
     * @brief 构造回调函数异步任务
     * @param callback 需要异步执行的函数
     * @param name 异步任务名称
     */
    SimpleTask(const CallbackType& callback, const std::string& name = std::string());

    virtual ~SimpleTask();

    void Run() override;

private:
    const CallbackType m_callback;
};
} /* namespace threading */