#include "fiber_recursive_mutex.h"

namespace threading
{
FiberRecursiveMutex::FiberRecursiveMutex()
    : m_lock(std::make_unique<boost::fibers::recursive_mutex>())
{
}

FiberRecursiveMutex::~FiberRecursiveMutex() = default;

void FiberRecursiveMutex::Lock()
{
    m_lock->lock();
}

void FiberRecursiveMutex::Unlock()
{
    m_lock->unlock();
}

bool FiberRecursiveMutex::TryLock()
{
    return m_lock->try_lock();
}
} /* namespace threading */
