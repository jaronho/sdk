#include "fiber_mutex.h"

namespace threading
{
FiberMutex::FiberMutex() : m_locker(std::make_unique<boost::fibers::mutex>()) {}

void FiberMutex::lock()
{
    m_locker->lock();
}

void FiberMutex::unlock()
{
    m_locker->unlock();
}

bool FiberMutex::tryLock()
{
    return m_locker->try_lock();
}
} // namespace threading
