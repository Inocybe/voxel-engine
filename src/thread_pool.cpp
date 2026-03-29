#include <thread_pool.hpp>


int ThreadPool::getThreadCount() const {
    return threads.size();
}
void ThreadPool::clearThreads() {
    threads.clear();
}