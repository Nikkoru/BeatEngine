#pragma once

template<typename T>
class Vector2 {
public:
    T X{};
    T Y{};
public:
    constexpr Vector2() = default;
    constexpr Vector2(T x, T y);

    constexpr Vector2<T> operator*=(Vector2<T> right);
    constexpr Vector2<T> operator*=(T right);
    constexpr Vector2<T> operator/=(Vector2<T> right);
    constexpr Vector2<T> operator/=(T right);
    constexpr Vector2<T> operator+=(Vector2<T> right);
    constexpr Vector2<T> operator-=(Vector2<T> right);
    constexpr bool operator==(Vector2<T> right);
    constexpr bool operator!=(Vector2<T> right);
};

using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;
using Vector2u = Vector2<unsigned int>;

template<typename T>
constexpr Vector2<T> operator-(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr Vector2<T> operator+(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr Vector2<T> operator*(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr Vector2<T> operator*(Vector2<T> left, T right);
template<typename T>
constexpr Vector2<T> operator*(T left, Vector2<T> right);
template<typename T>
constexpr Vector2<T> operator/(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr Vector2<T> operator/(Vector2<T> left, T right);
template<typename T>
constexpr Vector2<T> operator/(T left, Vector2<T> right);

#include "BeatEngine/Graphics/Vector2.inl"
