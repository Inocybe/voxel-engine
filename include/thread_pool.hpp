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
    Thread(ThreadPool* pool) : pool(pool) {
        thread = std::thread(&Thread::pool_worker, this);
    }
    ~Thread() {
        if (thread.joinable()) {
            thread.join();
        }
    }

private:
    std::thread thread;
    ThreadPool* pool; // pointer to the thread pool that this thread belongs to

    bool operation_completed = false; // flag to indicate if the operation is completed

    // function that the thread will execute, it will wait for tasks and execute them
    void pool_worker() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(pool->taskMutex);
                pool->newTaskCV.wait(lock, [this] { return !pool->tasks.empty(); }); // wait until there is a task

                task = std::move(pool->tasks.front());
                pool->tasks.pop();
            }

            // execute the task function with the provided arguments
            task.func(task.args);
        }
    }
};



class ThreadPool {
public:
    std::condition_variable newTaskCV;

    std::queue<Task> tasks;
    std::mutex taskMutex;


    ThreadPool() = default;
    ThreadPool(unsigned int maxThreads) : maxThreads(maxThreads) {
        for (unsigned int i = 0; i < maxThreads; ++i) {
            std::unique_ptr<Thread> thread = std::make_unique<Thread>(this);
            threads.emplace_back(std::move(thread));
        }
    }
    //~ThreadPool();

    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args) {
        // 1. Create a "wrapper" lambda that captures the logic
        // This matches the signature: std::function<void(std::array<std::any, 10>&)>
        auto taskFunc = [f = std::forward<Callable>(func)](std::array<std::any, 10>& providedArgs) mutable {
            // In a real scenario, you'd need to unpack 'providedArgs' here
            // or just capture the args directly in the lambda:
        };

        // 2. Alternatively, and much more simply for Thread Pools:
        // Capture the arguments directly in the lambda so you don't have to 
        // manually manage the std::any array inside the worker thread.
        auto boundTask = [f = std::forward<Callable>(func), 
                        ...capturedArgs = std::forward<Args>(args)]() mutable {
            f(capturedArgs...);
        };


        // add task to the queue
        std::lock_guard<std::mutex> lock(taskMutex);


        tasks.push(Task{taskFunc, boundTask});

        newTaskCV.notify_one(); // notify one thread that a new task is available
    }
    
    size_t getThreadCount() const {
        return threads.size();
    }
    unsigned int getMaxThreads() const {
        return maxThreads;
    }
private:
    std::vector<std::unique_ptr<Thread>> threads;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1; // leave one thread for main thread
};