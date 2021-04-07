#pragma once

#include <boost/fiber/all.hpp>
#include <memory>

namespace threading
{
/**
 * @brief 协程非递归锁
 * @note 正常情况下使用该锁即可, 如果在同一个协程栈中, 会多次上锁, 请使用递归锁`FiberRecursiveMutex`
 */
class FiberMutex
{
public:
    /**
     * @brief 构造
     */
    FiberMutex();

    /**
     * @brief 析构
     */
    ~FiberMutex();

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
    std::unique_ptr<boost::fibers::mutex> m_lock; /* 内部使用 boost 的 fiber 锁实现 */
};
} /* namespace threading */
