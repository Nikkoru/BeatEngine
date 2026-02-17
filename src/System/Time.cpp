#include "BeatEngine/System/Time.h"
#include <chrono>
#include <cstdint>


constexpr int32_t Time::AsMilliseconds() const {
    return std::chrono::duration_cast<std::chrono::duration<std::int32_t, std::milli>>(m_Duration).count();
}

constexpr int64_t Time::AsMicroseconds() const {
    return m_Duration.count();
}

constexpr std::chrono::microseconds Time::GetDuration() const {
    return m_Duration;
}

constexpr Time Time::FromSeconds(float seconds) {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(seconds));
}

constexpr Time Time::FromMilliseconds(int32_t milliseconds) {
    return std::chrono::milliseconds(milliseconds);
}

constexpr Time Time::FromMicroseconds(int64_t microseconds) {
    return std::chrono::microseconds(microseconds);
}
