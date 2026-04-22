#pragma once

#include <any>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <condition_variable>


class ThreadPool; // forward declaration


struct Task {
    std::function<void(std::array<std::any, 10>&)> func; // function to be executed, takes a vector of any type as arguments
    std::array<std::any, 10> args; // up to 10 arguments, can be any type
};

class Thread {
public:
    Thread(ThreadPool* pool);
    ~Thread();

private:
    std::thread thread;
    ThreadPool* pool; // pointer to the thread pool that this thread belongs to

    bool operation_completed = false; // flag to indicate if the operation is completed

    void pool_worker(); // function that the thread will execute, it will wait for tasks and execute them
};



class ThreadPool {
public:
    std::condition_variable newTaskCV;

    std::queue<Task> tasks;
    std::mutex taskMutex;


    ThreadPool() = default;
    ThreadPool(unsigned int maxThreads);
    //~ThreadPool();

    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args);
    size_t getThreadCount() const;
    unsigned int getMaxThreads() const;
private:
    std::vector<Thread> threads;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1; // leave one thread for main thread
};