#include "fiber_mutex.h"

namespace threading
{
FiberMutex::FiberMutex()
    : m_lock(std::make_unique<boost::fibers::mutex>())
{
}

FiberMutex::~FiberMutex() = default;

void FiberMutex::Lock()
{
    m_lock->lock();
}

void FiberMutex::Unlock()
{
    m_lock->unlock();
}

bool FiberMutex::TryLock()
{
    return m_lock->try_lock();
}
} /* namespace threading */
