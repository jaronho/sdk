#pragma once

#include <boost/fiber/all.hpp>
#include <memory>

namespace threading
{
/**
 * @brief 协程递归锁
 * @note 如果在同一个协程栈中, 会多次上锁, 请使用这个锁
 */
class FiberRecursiveMutex final
{
public:
    FiberRecursiveMutex();

    virtual ~FiberRecursiveMutex() = default;

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
    std::unique_ptr<boost::fibers::recursive_mutex> m_lock; /* 内部使用boost的fiber锁实现 */
};
} // namespace threading
