#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(size_t count = 4);
    ~ThreadPool(void);
    template<class F, class... Args>
    std::future<typename std::result_of<F(Args...)>::type> add(F&& f, Args&&... args);
    size_t totalCount(void);
    size_t idleCount(void);
    
private:
    std::vector<std::thread> mPool;                 /* need to keep track of threads so we can join them */
    std::queue<std::function<void(void)>> mTasks;   /* the task queue */
    std::mutex mQueueMutex;                         /* synchronization */
    std::condition_variable mCondition;
    bool mStop;
    size_t mTotalCount;
    size_t mIdleCount;
};

/* the constructor just launches some amount of workers */
inline ThreadPool::ThreadPool(size_t count) : mStop(false) {
    mTotalCount = count;
    mIdleCount = count;
    for (size_t i = 0; i < count; ++i) {
        mPool.emplace_back([this]{
            while (!this->mStop) {
                std::unique_lock<std::mutex> lock(this->mQueueMutex);
                this->mCondition.wait(lock, [this] {
                    return this->mStop || !this->mTasks.empty();
                });
                if (this->mStop && this->mTasks.empty()) {
                    return;
                }
                std::function<void(void)> task = std::move(this->mTasks.front());
                this->mTasks.pop();
                --mIdleCount;
                task();
                ++mIdleCount;
            }
        });
    }
}

/* the destructor joins all threads */
inline ThreadPool::~ThreadPool(void) {
    std::unique_lock<std::mutex> lock(mQueueMutex);
    mStop = true;
    mCondition.notify_all();
    for (std::thread& worker : mPool) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

/* add new work item to the pool */
template<class F, class... Args>
std::future<typename std::result_of<F(Args...)>::type> ThreadPool::add(F&& f, Args&&... args) {
    /* don't allow enqueueing after stopping the pool */
    if (mStop) {
        throw std::runtime_error("add on ThreadPool is stopped.");
    }
    using ReturnType = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<ReturnType> res = task->get_future();
    std::unique_lock<std::mutex> lock(mQueueMutex);
    mTasks.emplace([task]() {
        (*task)();
    });
    mCondition.notify_one();
    return res;
}

size_t ThreadPool::totalCount(void) {
    return mTotalCount;
}

size_t ThreadPool::idleCount(void) {
    return mIdleCount;
}

#endif  /* _THREAD_POOL_H_ */
