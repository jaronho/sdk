#include "fiber_recursive_mutex.h"

namespace threading
{
FiberRecursiveMutex::FiberRecursiveMutex() : m_locker(std::make_unique<boost::fibers::recursive_mutex>()) {}

void FiberRecursiveMutex::lock()
{
    m_locker->lock();
}

void FiberRecursiveMutex::unlock()
{
    m_locker->unlock();
}

bool FiberRecursiveMutex::tryLock()
{
    return m_locker->try_lock();
}
} // namespace threading
