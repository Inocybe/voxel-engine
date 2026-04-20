#pragma once

#include <any>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <condition_variable>

struct Task {
    std::function<void(std::vector<std::any>&)> func; // function to be executed, takes a vector of any type as arguments
    std::array<std::any, 10> args; // up to 10 arguments, can be any type
};

class Thread {
public:
    Thread() = default;
    template<typename Callable, typename... Args>
    Thread(Callable&& func, Args&&... args);
    ~Thread();

private:
    std::thread thread;

    bool operation_completed = false; // flag to indicate if the operation is completed
};



class ThreadPool {
public:
    std::condition_variable newTaskCV;

    std::queue<Task> tasks;
    std::mutex taskMutex;


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