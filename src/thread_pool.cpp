#include <thread_pool.hpp>
#include <iostream>



Thread::Thread(ThreadPool* pool) : pool(pool) {
    thread = std::thread(&Thread::pool_worker, this);
}

Thread::~Thread() {
    if (thread.joinable()) {
        thread.join();
    }
}

void Thread::pool_worker() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(pool->taskMutex);
            pool->newTaskCV.wait(lock, [this] { return pool->stop.load() || !pool->tasks.empty(); });

            if (pool->stop.load() && pool->tasks.empty()) {
                return;
            }

            task = std::move(pool->tasks.front());
            //std::cout << "Thread " << std::this_thread::get_id() << " executing task\n";
            pool->tasks.pop();
        }

        try {
            task.func();
        } catch (const std::exception& e) {
            std::cerr << "task exception: " << e.what() << '\n';
        } catch (...) {
            std::cerr << "task unknown exception\n";
        }
    }
}

ThreadPool::ThreadPool(unsigned int maxThreads) : maxThreads(maxThreads) {
    for (unsigned int i = 0; i < maxThreads; ++i) {
        threads.emplace_back(std::make_unique<Thread>(this));
    }
}

ThreadPool::~ThreadPool() {
    stop.store(true);
    newTaskCV.notify_all();
    threads.clear();
}

size_t ThreadPool::getThreadCount() const {
    return threads.size();
}

unsigned int ThreadPool::getMaxThreads() const {
    return maxThreads;
}