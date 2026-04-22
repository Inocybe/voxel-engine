#include <thread_pool.hpp>


// THREAD IMPLEMENTATION
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
            pool->newTaskCV.wait(lock, [this] { return !pool->tasks.empty(); }); // wait until there is a task

            task = std::move(pool->tasks.front());
            pool->tasks.pop();
        }

        // execute the task function with the provided arguments
        task.func(task.args);
    }
}

/* TODO:
I need to create a function that will make. the thread sleep until it is 
notified by the condition vairble

when it is notified, it calls a function that is while true
while (true) {
    first check if the tasks que is empty, if it is, skip checking the CV
    do the cv thing, so it sleeps, when until notified
    does the function that is inside the task, make sure to use mutex
}

*/






// THREADPOOL IMPLEMENTATION
ThreadPool::ThreadPool(unsigned int maxThreads) : maxThreads(maxThreads) {
    for (unsigned int i = 0; i < maxThreads; ++i) {
        threads.emplace_back(Thread(this));
    }
}
//ThreadPool::~ThreadPool() { }


template<typename Callable, typename... Args>
void ThreadPool::addTask(Callable&& func, Args&&... args) {
    // add task to the queue
    std::lock_guard<std::mutex> lock(taskMutex);
    tasks.push(Task{std::forward<Callable>(func), std::array<std::any, 10>{std::forward<Args>(args)...}});

    newTaskCV.notify_one(); // notify one thread that a new task is available
}

size_t ThreadPool::getThreadCount() const {
    return threads.size();
}

unsigned int ThreadPool::getMaxThreads() const {
    return maxThreads;
}
