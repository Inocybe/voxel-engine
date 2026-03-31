#pragma once
#include "Thread.hpp"
#include <vector>
#include <stdexcept>

// ============================================================
// ThreadPool.hpp
// Owns a collection of Thread objects and lets you add tasks.
//
// When the ThreadPool is destroyed, every Thread in the vector
// is destroyed, which triggers each Thread's destructor, which
// calls join() on each OS thread. You get automatic cleanup
// for free through the RAII chain.
// ============================================================

class ThreadPool {
public:

    // Adds a new task as a real OS thread.
    //
    // Same template pattern as Thread's constructor -- works with
    // any callable and any arguments.
    //
    // threads.emplace_back(...) constructs a Thread directly inside
    // the vector without making a temporary copy first.
    // This is preferred over push_back() which would construct a
    // temporary and then move/copy it in -- emplace_back skips that.
    //
    // std::ref(engine) is how you pass something BY REFERENCE into
    // a thread. You cannot just use & like in a normal function call
    // because the thread internally stores and copies its arguments.
    // std::ref wraps the reference so it survives that copy.
    template<typename Callable, typename... Args>
    void addTask(Callable&& func, Args&&... args) {
        if (threads.size() >= maxThreads) {
            throw std::runtime_error(
                "ThreadPool: max thread count reached. "
                "Wait for threads to finish or increase maxThreads."
            );
        }
        threads.emplace_back(
            std::forward<Callable>(func),
            std::forward<Args>(args)...
        );
    }

    // Returns how many threads are currently alive in the pool.
    size_t threadCount() const {
        return threads.size();
    }

    // Returns the maximum number of threads this pool allows.
    unsigned int getMaxThreads() const {
        return maxThreads;
    }

private:

    // The collection of active threads.
    // std::vector<Thread> means each element is a Thread object.
    // When the vector is destroyed, every Thread destructor runs,
    // which joins every OS thread automatically.
    std::vector<Thread> threads;

    // hardware_concurrency() returns the number of logical CPU cores
    // on the machine. We subtract 1 to leave one core free for the
    // main/render thread. This is a reasonable default -- you don't
    // want all cores maxed out by worker threads with nothing left
    // for the thread that's drawing frames and handling input.
    //
    // NOTE: hardware_concurrency() can return 0 on some platforms
    // if it can't determine the core count. A production codebase
    // would guard against this:
    //   unsigned int cores = std::thread::hardware_concurrency();
    //   maxThreads = (cores > 1) ? cores - 1 : 1;
    unsigned int maxThreads = std::thread::hardware_concurrency() - 1;
};
