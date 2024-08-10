#pragma once

#include <chrono>

struct Throttle
{
    void set_frame_rate(double frame_rate);
    void advance();
    void reset();
    void wait_for_frame();
    void wait_for_frame_and_rebase_time();
    std::chrono::microseconds remaining();
    double max_frame_rate = 0.0;
    double frame_duration = 0.0;
    std::chrono::microseconds frame_duration_us;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::microseconds> then;
};