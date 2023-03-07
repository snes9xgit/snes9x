#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <future>
#include <condition_variable>

class threadpool
{
public:
    void start(int num_threads);
    void stop();
    void queue(std::function<void()> func);
    void wait_idle();

private:
    void thread_func();

    std::vector<std::thread> threads;
    std::queue<std::packaged_task<void()>> jobs;
    std::queue<std::shared_future<void>> futures;
    std::mutex mutex;
    std::condition_variable cond;
    bool die = false;
    bool started = false;
};