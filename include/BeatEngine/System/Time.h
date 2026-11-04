#pragma once

#include <chrono>
#include <cstdint>

#ifdef _MSC_VER
#include <compare>
#endif

class Time {
private:
    std::chrono::microseconds m_Duration{};
public:
    constexpr Time() = default;
    
    template<typename Rep, typename Period>
    constexpr Time(const std::chrono::duration<Rep, Period>& duration);

    constexpr float AsSeconds() const;
    constexpr float AsMilliseconds() const;
    constexpr int64_t AsMicroseconds() const;
    constexpr std::chrono::time_point<std::chrono::system_clock> AsTimePoint() const;
    constexpr time_t AsTimeT() const;
    constexpr std::chrono::microseconds GetDuration() const;
    
    template<typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const;

    static constexpr Time FromSeconds(float seconds);
    static constexpr Time FromMilliseconds(int32_t milliseconds);
    static constexpr Time FromMicroseconds(int64_t microseconds);

    constexpr auto operator<=>(const Time& right) const = default;
    constexpr Time operator-(const Time& right) const { return m_Duration - right.m_Duration; }
    constexpr Time operator+(const Time& right) const { return m_Duration + right.m_Duration; }

    template<typename Rep, typename Period>
    constexpr Time operator-(const std::chrono::duration<Rep, Period>& right) const { return m_Duration - right; }
    template<typename Rep, typename Period>
    constexpr Time operator+(const std::chrono::duration<Rep, Period>& right) const { return m_Duration + right; }
};

template<typename Rep, typename Period>
Time operator-(const std::chrono::duration<Rep, Period>& left, const Time& right) { return left - right.GetDuration(); }
template<typename Rep, typename Period>
Time operator+(const std::chrono::duration<Rep, Period>& left, const Time& right) { return left + right.GetDuration(); }

#include "BeatEngine/System/Time.inl"
