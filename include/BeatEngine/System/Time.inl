#include "BeatEngine/System/Time.h"

constexpr float Time::AsSeconds() const {
    return std::chrono::duration<float>(m_Duration).count();
}

template<typename Rep, typename Period>
constexpr Time::Time(const std::chrono::duration<Rep, Period>& duration)
    : m_Duration(duration) {
}

template<typename Rep, typename Period>
constexpr Time::operator std::chrono::duration<Rep,Period>() const {
    return m_Duration;
}
