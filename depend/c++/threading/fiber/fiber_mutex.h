#pragma once

#include <boost/fiber/all.hpp>
#include <memory>

namespace threading
{
/**
 * @brief 协程非递归锁
 * @note 正常情况下使用该锁即可, 如果在同一个协程栈中, 会多次上锁, 请使用递归锁FiberRecursiveMutex
 */
class FiberMutex final
{
public:
    FiberMutex();

    virtual ~FiberMutex() = default;

    /**
     * @brief 上锁
     */
    void lock();

    /**
     * @brief 解锁
     */
    void unlock();

    /**
     * @brief 尝试上锁
     * @return true-上锁成功, false-失败
     */
    bool tryLock();

private:
    std::unique_ptr<boost::fibers::mutex> m_lock; /* 内部使用boost的fiber锁实现 */
};
} /* namespace threading */
