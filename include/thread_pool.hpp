#pragma once

#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <tuple>
#include <utility>
#include <memory>


struct Task {
    std::function<void()> func;
};

class ThreadPool;
class Thread;

class Thread {
public:
    Thread(ThreadPool* pool);
    ~Thread();

private:
    std::thread thread;
    ThreadPool* pool = nullptr;
    bool operation_completed = false;
    void pool_worker();
};

class ThreadPool {
public:
    std::condition_variable newTaskCV;
    std::queue<Task> tasks;
    std::mutex taskMutex;
    std::atomic<bool> stop{false};



    ThreadPool(unsigned int maxThreads);
    ~ThreadPool();

    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args) {
        auto taskFunc = [f = std::forward<Callable>(func),
                         argsTuple = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            std::apply(f, argsTuple);
        };

        Task t;
        t.func = std::move(taskFunc);

        {
            std::lock_guard<std::mutex> lock(taskMutex);
            tasks.push(std::move(t));
        }

        newTaskCV.notify_one();
    }

    size_t getThreadCount() const;
    unsigned int getMaxThreads() const;

private:
    std::vector<std::unique_ptr<Thread>> threads;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1;
};