#include "BeatEngine/System/Time.h"

constexpr float Time::AsSeconds() const {
    return std::chrono::duration<float>(m_Duration).count();
}

constexpr int32_t Time::AsMilliseconds() const {
    return std::chrono::duration_cast<std::chrono::duration<std::int32_t, std::milli>>(m_Duration).count();
}

constexpr int64_t Time::AsMicroseconds() const {
    return m_Duration.count();
}

constexpr std::chrono::time_point<std::chrono::system_clock> Time::AsTimePoint() const {
    return std::chrono::system_clock::time_point(m_Duration);
}

constexpr time_t Time::AsTimeT() const {
    return std::chrono::system_clock::to_time_t(AsTimePoint());
}

constexpr std::chrono::microseconds Time::GetDuration() const {
    return m_Duration;
}

template<typename Rep, typename Period>
constexpr Time::Time(const std::chrono::duration<Rep, Period>& duration)
    : m_Duration(duration) {
}

template<typename Rep, typename Period>
constexpr Time::operator std::chrono::duration<Rep,Period>() const {
    return m_Duration;
}
