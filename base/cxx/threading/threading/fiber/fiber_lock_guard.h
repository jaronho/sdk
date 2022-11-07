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
     * @param locker 对应锁的对象
     */
    FiberLockGuard(LockType& locker) : m_locker(locker)
    {
        m_locker.Lock();
    }

    /**
     * @brief 析构函数, 释放对应锁
     */
    ~FiberLockGuard()
    {
        m_locker.Unlock();
    }

private:
    LockType& m_locker; /* 锁对象 */
};
} // namespace threading
