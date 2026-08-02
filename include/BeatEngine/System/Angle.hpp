#pragma once

#ifdef _MSC_VER
#include <compare>
#endif

class Angle {
private:
    float m_Radians{};
public:
    static const Angle Zero;
public:
    constexpr Angle() = default;

    constexpr float AsDegrees() const;
    constexpr float AsRadians() const;

    static constexpr Angle FromDegrees(float angle);
    static constexpr Angle FromRadians(float angle);

#ifndef _MSC_VER
    constexpr bool operator<=>(const Angle& other) const = default;
#else
	constexpr std::partial_ordering operator<=>(const Angle& other) const = default;
#endif

    constexpr Angle operator-();

    constexpr Angle operator+(Angle other);
    constexpr Angle operator-(Angle other);
    constexpr Angle operator*(Angle other);
    constexpr Angle operator*(float value);
    constexpr Angle operator/(Angle other);
    constexpr Angle operator/(float value);
    constexpr Angle operator%(Angle other);

    constexpr Angle& operator+=(Angle other);
    constexpr Angle& operator-=(Angle other);
    constexpr Angle& operator*=(Angle other);
    constexpr Angle& operator*=(float value);
    constexpr Angle& operator/=(Angle other);
    constexpr Angle& operator/=(float value);
    constexpr Angle& operator%=(Angle other);
private:
    constexpr Angle(float radians);
};

#include "BeatEngine/System/Angle.inl"
