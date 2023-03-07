#include "threadpool.h"
#include <chrono>

void threadpool::thread_func()
{
    while (1)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait_for(lock, std::chrono::microseconds(100), [this] {
            return (die || !jobs.empty());
        });

        if (die)
            return;

        if (jobs.empty())
            continue;
            
        auto job = std::move(jobs.front());
        jobs.pop();
        lock.unlock();

        job();        
    }
}

void threadpool::start(int num_threads)
{
    die = false;

    while (!jobs.empty())
        jobs.pop();
    
    threads.resize(num_threads);

    for (int i = 0; i < num_threads; i++)
        threads.at(i) = std::thread(&threadpool::thread_func, this);

    started = true;
}

void threadpool::wait_idle()
{
    while (!futures.empty())
    {
        futures.front().wait();
        std::unique_lock<std::mutex> lock(mutex);    
        futures.pop();
    }
}

void threadpool::stop()
{
    if (!started)
        return;

    std::unique_lock<std::mutex> lock(mutex);
    die = true;
    lock.unlock();

    cond.notify_all();
    for (auto &t : threads)
        t.join();
    threads.clear();

    while (!jobs.empty())
        jobs.pop();

    started = false;
}

void threadpool::queue(std::function<void()> func)
{
    std::unique_lock<std::mutex> lock(mutex);

    std::packaged_task<void()> job(func);

    futures.push(job.get_future().share());
    jobs.push(std::move(job));
    
    lock.unlock();

    cond.notify_all();
    return;
}