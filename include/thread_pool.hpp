#pragma once


#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>

#include <world.hpp>


class Thread {
public:
    Thread() = default;
    template<typename Callable, typename... Args>
    Thread(Callable&& func, Args&&... args);
    ~Thread();
private:
    std::thread thread;
};



class ThreadPool {
public:
    ThreadPool() = default;
    ThreadPool(unsigned int maxThreads);

    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args);
    size_t getThreadCount() const;
    unsigned int getMaxThreads() const;
private:
    std::vector<Thread> threads;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1; // leave one thread for main thread
};