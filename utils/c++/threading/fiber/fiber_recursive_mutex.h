#pragma once

#include <boost/fiber/all.hpp>
#include <memory>

namespace threading
{
/**
 * @brief 协程递归锁
 * @note 如果在同一个协程栈中, 会多次上锁, 请使用这个锁
 */
class FiberRecursiveMutex
{
public:
    /**
     * @brief 构造
     */
    FiberRecursiveMutex();

    /**
     * @brief 析构
     */
    ~FiberRecursiveMutex();

    /**
     * @brief 上锁
     */
    void Lock();

    /**
     * @brief 解锁
     */
    void Unlock();

    /**
     * @brief 尝试上锁
     * @return 上锁成功则返回true, 否则返回false
     */
    bool TryLock();

private:
    std::unique_ptr<boost::fibers::recursive_mutex> m_lock; /* 内部使用 boost 的 fiber 锁实现 */
};
} /* namespace threading */
