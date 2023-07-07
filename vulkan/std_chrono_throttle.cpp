#include "std_chrono_throttle.hpp"
#include <thread>

using namespace std::chrono;

void Throttle::set_frame_rate(double frame_rate)
{
    max_frame_rate = frame_rate;
    frame_duration = 1.0 / max_frame_rate;
    frame_duration_us = microseconds(int64_t(frame_duration * 1000000.0));
}

microseconds Throttle::remaining()
{
    auto now = time_point_cast<microseconds>(steady_clock::now());
    auto diff = (now - then);
    return frame_duration_us - diff;
}

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void Throttle::wait_for_frame()
{
        static HANDLE timer = nullptr;

    if (timer == nullptr)
        timer = CreateWaitableTimer(nullptr, true, nullptr);

    auto time_to_wait = remaining();

    if (time_to_wait < -frame_duration_us / 10)
        reset();

    if (time_to_wait.count() > 2000)
    {
        LARGE_INTEGER li;
        li.QuadPart = -(time_to_wait.count() - 2000) * 10;
        SetWaitableTimer(timer, &li, 0, nullptr, nullptr, false);
        WaitForSingleObject(timer, INFINITE);
    }

    time_to_wait = remaining();
    while (time_to_wait.count() > 0)
    {
        std::this_thread::yield();
        time_to_wait = remaining();
    }
}
#endif

#if !defined(_WIN32)
void Throttle::wait_for_frame()
{
    auto time_to_wait = remaining();

    if (time_to_wait < -frame_duration_us / 10)
        reset();

    if (time_to_wait.count() > 1000)
        std::this_thread::sleep_for(time_to_wait - 1ms);

    time_to_wait = remaining();
    while (time_to_wait.count() > 0)
    {
        std::this_thread::yield();
        time_to_wait = remaining();
    }
}
#endif

void Throttle::wait_for_frame_and_rebase_time()
{
    wait_for_frame();
    advance();
}

void Throttle::advance()
{
    then += frame_duration_us;
}

void Throttle::reset()
{
    auto now = time_point_cast<microseconds>(steady_clock::now());
    then = now;
}
