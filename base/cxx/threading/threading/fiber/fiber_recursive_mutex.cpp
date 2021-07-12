#include "fiber_recursive_mutex.h"

namespace threading
{
FiberRecursiveMutex::FiberRecursiveMutex() : m_lock(std::make_unique<boost::fibers::recursive_mutex>()) {}

void FiberRecursiveMutex::lock()
{
    m_lock->lock();
}

void FiberRecursiveMutex::unlock()
{
    m_lock->unlock();
}

bool FiberRecursiveMutex::tryLock()
{
    return m_lock->try_lock();
}
} // namespace threading
