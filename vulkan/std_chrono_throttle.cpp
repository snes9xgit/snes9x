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

void Throttle::wait_for_frame()
{
    auto time_to_wait = remaining();

    if (time_to_wait.count() > 0)
        std::this_thread::sleep_for(time_to_wait);
}

void Throttle::wait_for_frame_and_rebase_time()
{
    auto time_to_wait = remaining();

    if (time_to_wait < -frame_duration_us / 10)
        reset();

    if (time_to_wait.count() > 0)
    {
        std::this_thread::sleep_for(time_to_wait);
        advance();
    }
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
