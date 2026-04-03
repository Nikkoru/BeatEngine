#include "BeatEngine/System/Time.h"
#include "BeatEngine/System/Clock.h"
#include <chrono>
#include <cstdint>



constexpr Time Time::FromSeconds(float seconds) {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(seconds));
}

constexpr Time Time::FromMilliseconds(int32_t milliseconds) {
    return std::chrono::milliseconds(milliseconds);
}

constexpr Time Time::FromMicroseconds(int64_t microseconds) {
    return std::chrono::microseconds(microseconds);
}
