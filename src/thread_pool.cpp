#include <thread_pool.hpp>


// THREAD IMPLEMENTATION
template<typename Callable, typename... Args>
Thread::Thread(Callable&& func, Args&&... args) {
    thread = std::thread(std::forward<Callable>(func), std::forward<Args>(args)...);
}
Thread::~Thread() {
    if (thread.joinable()) {
        thread.join();
    }
}


// THREADPOOL IMPLEMENTATION
ThreadPool::ThreadPool(unsigned int maxThreads) : maxThreads(maxThreads) {}

template<typename Callable, typename... Args>
void ThreadPool::addTask(Callable&& func, Args&&... args) {
    if (threads.size() >= maxThreads) {
        throw std::runtime_error("ThreadPool has reached its maximum thread count");
    }

    threads.emplace_back(std::forward<Callable>(func), std::forward<Args>(args)...);
}

size_t ThreadPool::getThreadCount() const {
    return threads.size();
}

unsigned int ThreadPool::getMaxThreads() const {
    return maxThreads;
}
