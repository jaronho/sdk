#pragma once

namespace threading
{
/**
 * @brief 符合RAII的上锁解锁器
 * @param LockType 锁类型
 */
template<typename LockType>
class FiberLockGuard final
{
public:
    /**
     * @brief 构造函数, 同时上锁
     * @param lock 对应锁的对象
     */
    FiberLockGuard(LockType& lock) : m_lock(lock)
    {
        m_lock.Lock();
    }

    /**
     * @brief 析构函数, 释放对应锁
     */
    ~FiberLockGuard()
    {
        m_lock.Unlock();
    }

private:
    LockType& m_lock; /* 锁对象 */
};
} // namespace threading
