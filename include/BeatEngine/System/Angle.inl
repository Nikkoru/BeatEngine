#include "BeatEngine/System/Angle.hpp"
#include "BeatEngine/Util/Math.h"
#include <cassert>

namespace {
constexpr float positiveRemainer(float a, float b) {
    assert(b > 0.f && "Unable to calc remainer with non-positive divisor");
    const float val = a - static_cast<float>(static_cast<int>(a / b)) * b;
    return val >= 0.f ? val : val + b;
} 
}
constexpr Angle Angle::Zero{};

constexpr float Angle::AsDegrees() const {
    return m_Radians * (180.f / PI);
}

constexpr float Angle::AsRadians() const {
    return m_Radians;
}

constexpr Angle Angle::FromDegrees(float angle) {
    return Angle{angle * (PI / 180.f)};
}

constexpr Angle Angle::FromRadians(float angle) {
    return Angle{angle};
}

constexpr Angle Angle::operator-() {
    return FromRadians(-m_Radians);
}

constexpr Angle Angle::operator+(Angle other) {
    return FromRadians(m_Radians + other.m_Radians);
}

constexpr Angle Angle::operator-(Angle other) {
    return FromRadians(m_Radians - other.m_Radians);
}

constexpr Angle Angle::operator*(Angle other) {
    return FromRadians(m_Radians * other.m_Radians);
}

constexpr Angle Angle::operator*(float value) {
    return FromRadians(m_Radians * value);
}

constexpr Angle Angle::operator/(Angle other) {
    assert(other.m_Radians != 0.f && "divide by 0 not allowed");
    return FromRadians(m_Radians / other.m_Radians);
}

constexpr Angle Angle::operator/(float value) {
    assert(value != 0.f && "divide by 0 not allowed");
    return FromRadians(m_Radians / value);
}

constexpr Angle Angle::operator%(Angle other) {
    return FromRadians(positiveRemainer(m_Radians, other.m_Radians));
}

constexpr Angle& Angle::operator+=(Angle other) {
    return *this = *this + other;
}

constexpr Angle& Angle::operator-=(Angle other) {
    return *this = *this - other;
}

constexpr Angle& Angle::operator*=(Angle other) {
    return *this = *this * other;
}

constexpr Angle& Angle::operator*=(float value) {
    return *this = *this * value;
}

constexpr Angle& Angle::operator/=(Angle other) {
    return *this = *this / other;
}

constexpr Angle& Angle::operator/=(float value) {
    return *this = *this / value;
}

constexpr Angle& Angle::operator%=(Angle other) {
    return *this = *this % other;
}

constexpr Angle::Angle(float radians) : m_Radians(radians) {}
