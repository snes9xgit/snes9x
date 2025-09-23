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
    static UINT (WINAPI *NtDelayExecution)(BOOL, LARGE_INTEGER *) = nullptr;
    static UINT (WINAPI *NtQueryTimerResolution) (ULONG *, ULONG *, ULONG *) = nullptr;
    static UINT (WINAPI *NtSetTimerResolution) (ULONG, BOOL, ULONG *) = nullptr;
    static int timer_resolution = 12500;

    if (NtDelayExecution == nullptr)
    {
        HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");

        if (ntdll)
        {
            NtDelayExecution = reinterpret_cast<typeof NtDelayExecution>(GetProcAddress(ntdll, "NtDelayExecution"));
            NtQueryTimerResolution = reinterpret_cast<typeof NtQueryTimerResolution>(GetProcAddress(ntdll, "NtQueryTimerResolution"));
            NtSetTimerResolution   = reinterpret_cast<typeof NtSetTimerResolution>(GetProcAddress(ntdll, "NtSetTimerResolution"));
        }

        if (NtQueryTimerResolution)
        {
            ULONG min, max, current;
            NtQueryTimerResolution(&min, &max, &current);

            if (NtSetTimerResolution)
                NtSetTimerResolution(max, true, &current);

            timer_resolution = current * 5 / 4;
        }
    }

    auto time_to_wait = remaining();

    if (time_to_wait < -(frame_duration_us / 8))
    {
        reset();
        return;
    }

    if (NtDelayExecution)
    {
        if (time_to_wait.count() * 10 > timer_resolution)
        {
            LARGE_INTEGER li;
            li.QuadPart = -(time_to_wait.count() * 10 - timer_resolution);
            NtDelayExecution(false, &li);
        }
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

    if (time_to_wait < -frame_duration_us)
    {
        reset();
        return;
    }

    if (time_to_wait.count() > 1000)
        std::this_thread::sleep_for(time_to_wait - 1000us);

    time_to_wait = remaining();
    while (time_to_wait.count() > 0)
    {
        std::this_thread::yield();
        time_to_wait = remaining();
    }
}
#endif

double Throttle::get_late_frames()
{
    if (frame_duration_us == 0us)
        return 0.0;
    return remaining() / -frame_duration_us;
}

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
