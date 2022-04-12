#include "fiber_mutex.h"

namespace threading
{
FiberMutex::FiberMutex() : m_lock(std::make_unique<boost::fibers::mutex>()) {}

void FiberMutex::lock()
{
    m_lock->lock();
}

void FiberMutex::unlock()
{
    m_lock->unlock();
}

bool FiberMutex::tryLock()
{
    return m_lock->try_lock();
}
} // namespace threading
