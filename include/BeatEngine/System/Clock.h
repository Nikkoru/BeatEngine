#pragma once

#include <chrono>
#include <type_traits>
using ClockImpl = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock, std::chrono::steady_clock>;

class Time;
class Clock {
private:
    ClockImpl::time_point m_RefPoint{ClockImpl::now()};
    ClockImpl::time_point m_StopPoint{};
public:
    void Start();
    void Stop();

    bool IsRunning();
    
    Time Get();
    Time GetAndStop();
    Time GetAndReset();
};
