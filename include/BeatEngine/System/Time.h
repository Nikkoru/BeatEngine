#pragma once

#include <chrono>
#include <cstdint>
class Time {
private:
    std::chrono::microseconds m_Duration{};
public:
    constexpr Time() = default;
    
    template<typename Rep, typename Period>
    constexpr Time(const std::chrono::duration<Rep, Period>& duration);

    constexpr float AsSeconds() const;
    constexpr int32_t AsMilliseconds() const;
    constexpr int64_t AsMicroseconds() const;
    constexpr std::chrono::microseconds GetDuration() const;
    
    template<typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const;

    static constexpr Time FromSeconds(float seconds);
    static constexpr Time FromMilliseconds(int32_t milliseconds);
    static constexpr Time FromMicroseconds(int64_t microseconds);

    constexpr bool operator<=>(const Time& right) const = default;
};

#include "BeatEngine/System/Time.inl"
