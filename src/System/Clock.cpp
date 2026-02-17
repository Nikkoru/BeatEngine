#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/Time.h"
#include <chrono>

void Clock::Start() {
    if (!IsRunning()) {
        m_RefPoint += ClockImpl::now() - m_StopPoint;
        m_StopPoint = {};
    }
}

void Clock::Stop() {
    if (IsRunning())
        m_StopPoint = ClockImpl::now();
}

bool Clock::IsRunning() {
    return m_StopPoint == ClockImpl::time_point();
}

Time Clock::Get() {
    if (IsRunning())
        return std::chrono::duration_cast<std::chrono::microseconds>(ClockImpl::now() - m_RefPoint);
    else
        return std::chrono::duration_cast<std::chrono::microseconds>(m_StopPoint - m_RefPoint);
}

Time Clock::GetAndStop() {
    const Time elapsed = Get();
    m_RefPoint = ClockImpl::now();
    m_StopPoint = m_RefPoint;
    return elapsed;
}

Time Clock::GetAndReset() {
    const Time elapsed = Get();
    m_RefPoint = ClockImpl::now();
    m_StopPoint = {};
    return elapsed;
}
