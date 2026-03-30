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
    Thread(Callable&& func, Args&&... args) {
        thread = std::thread(std::forward<Callable>(func), std::forward<Args>(args)...);
    }

    ~Thread() {
        if (thread.joinable()) {
            thread.join();
        }
    } 
private:
    std::thread thread;
};



class ThreadPool {
public:
    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args) {
        threads.emplace_back(std::forward<Callable>(func), std::forward<Args>(args)...);
    }

    int getThreadCount() const;
    void clearThreads();
private:
    std::vector<Thread> threads;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1; // leave one thread for main thread
};