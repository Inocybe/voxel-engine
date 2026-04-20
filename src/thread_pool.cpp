#include <thread_pool.hpp>


// THREAD IMPLEMENTATION
template<typename Callable, typename... Args>
Thread::Thread(Callable&& func, Args&&... args) {
    thread = std::thread(std::forward<Callable>(func), std::forward<Args>(args)...);
}
Thread::~Thread() {
    if (thread.joinable()) {
        thread.join();
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

}

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
