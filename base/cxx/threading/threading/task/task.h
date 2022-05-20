#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#ifdef _WIN32
#define __THREADING_FILENAME__(x) strrchr(x, '\\') ? strrchr(x, '\\') + 1 : x
#else
#define __THREADING_FILENAME__(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x
#endif

/**
 * @brief 宏定义线程接口调用者(自动拼接: 文件名_行号_函数名)
 */
#define THREADING_CALLER \
    std::string(__THREADING_FILENAME__(__FILE__)).append("_").append(std::to_string(__LINE__)).append("_").append(__FUNCTION__)

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
        created, /* 已创建 */
        queuing, /* 排队中 */
        running, /* 运行中 */
        finished /* 已完成 */
    };

    /**
	 * @brief 构造函数
	 * @param name 任务名称(强烈建议设置唯一标识, 以方便后续诊断)
	 */
    Task(const std::string& name);

    virtual ~Task() = default;

    /**
     * @brief 获取ID
     * @return 任务ID
     */
    int64_t getId() const;

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
    int64_t m_id = 0; /* ID */
    const std::string m_name; /* 任务名称 */
    std::atomic<State> m_state = {State::created}; /* 任务状态 */
    std::atomic_bool m_cancelled = {false}; /* 是否已取消 */
    std::mutex m_mutex; /* for m_cv */
    std::condition_variable m_cv;
};

using TaskPtr = std::shared_ptr<Task>;
} // namespace threading
