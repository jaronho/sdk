#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace threading
{
/**
 * @brief 任务基类
 */
class Task
{
public:
    /**
	 * @brief 任务执行状态
	 */
    enum class State
    {
        CREATED, /* 已创建 */
        QUEUING, /* 排队中 */
        RUNNING, /* 运行中 */
        FINISHED /* 已完成 */
    };

    /**
	 * @brief 创建任务
	 * @param name 任务名称
	 */
    Task(const std::string& name);

    virtual ~Task() = default;

    /**
	 * @brief 获取名称
	 * @return 任务名称
	 */
    std::string getName() const;

    /**
	 * @brief 获取执行状态
	 * @return 执行状态
	 */
    State getState() const;

    /**
	 * @brief 设置执行状态
	 * @param state 执行状态
	 */
    void setState(const State& state);

    /**
	 * @brief 是否已取消
	 * @return true-取消, false-未取消
	 */
    bool isCancelled() const;

    /**
	 * @brief 取消任务
	 */
    virtual void cancel();

    /**
	 * @brief 等待正在执行的任务完成, 如果任务未在执行, 则立即返回
	 */
    void join();

    /**
	 * @brief 任务执行函数实现
	 */
    virtual void run() = 0;

private:
    const std::string m_name; /* 任务名称 */
    std::atomic<State> m_state = {State::CREATED}; /* 任务状态 */
    std::atomic_bool m_cancelled = {false}; /* 是否已取消 */
    std::mutex m_mutex; /* for m_cv */
    std::condition_variable m_cv;
};

using TaskPtr = std::shared_ptr<Task>;
} // namespace threading
