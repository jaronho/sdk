#pragma once
#include <chrono>
#include <functional>
#include <string>

namespace utilitiy
{
/**
 * @brief 观察回调函数
 * @param tag 标签
 * @param subTag 子标签
 * @param elapsed 时长(毫秒)
 */
using WatchFunc = std::function<void(const std::string& tag, const std::string& subTag, long long elapsed)>;

/**
 * @brief 结束回调函数
 * @param tag 标签
 * @param elapsed 时长(毫秒)
 */
using EndFunc = std::function<void(const std::string& tag, long long elapsed)>;

/**
 * @brief 时间观察者(注意:非线程安全)
 */
class TimeWatcher
{
public:
    /**
	 * @brief 构造函数
	 * @param watchFunc 观察函数
	 * @param endFunc 结束函数
	 * @param tag 标签
	 */
    TimeWatcher(const WatchFunc& watchFunc, const EndFunc& endFunc, const std::string& tag = "");
    TimeWatcher() = default;

    ~TimeWatcher();

    /**
	 * @brief 观察
	 * @param subTag 子标签
	 * @param watchFunc 观察函数(若有设置,则回调该函数,否则回调构造时所设置的函数)
	 */
    void watch(const std::string& subTag = "", const WatchFunc& watchFunc = nullptr);

    /**
	 * @brief 检测是否超时
	 * @param timeout 超时(毫秒)
	 * @return true-超时, false-不超时
	 */
    bool check(long long timeout);

private:
    std::chrono::steady_clock::time_point m_begin; /* 开始时间 */
    std::chrono::steady_clock::time_point m_watch; /* 观察时间 */
    WatchFunc m_watchFunc; /* 观察函数 */
    EndFunc m_endFunc; /* 结束函数 */
    std::string m_tag; /* 标签 */
};
} // namespace utilitiy

#define TIME_WATCHER_VAR_NAME_CONNECTION(var1, var2) var1##var2
#define TIME_WATCHER_VAR_NAME(var1, var2) TIME_WATCHER_VAR_NAME_CONNECTION(var1, var2)
#define TIME_WATCHER_VAR(var) TIME_WATCHER_VAR_NAME(var, __LINE__)

/* 创建时间观察者(非线程安全) */
#define MAKE_TIME_WATCHER(watcher, watchFunc, endFunc, tag) utilitiy::TimeWatcher watcher(watchFunc, endFunc, tag);

/* 时间观察者观察(非线程安全) */
#define TIME_WATCHER_WATCH(watcher, subTag) watcher.watch(subTag);

/* 时间观察者观察(非线程安全) */
#define TIME_WATCHER_CHECK(watcher, timeout) watcher.check(timeout);

/* 启动简单时间观察者(非线程安全,自动命名变量) */
#define START_SIMPLE_TIME_WATCHER(endFunc) MAKE_TIME_WATCHER(TIME_WATCHER_VAR(_timeWatcher_), nullptr, endFunc, "")
