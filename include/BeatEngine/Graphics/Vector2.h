#pragma once

// #include <imgui.h>

#include <glm/ext/vector_float2.hpp>
template<typename T>
class Vector2 {
public:
    T X{};
    T Y{};
public:
    constexpr Vector2() = default;
    constexpr Vector2(T x);
    constexpr Vector2(T x, T y);
    // constexpr Vector2(ImVec2 vec);

    constexpr Vector2<T> operator*=(Vector2<T> right);
    constexpr Vector2<T> operator*=(T right);
    constexpr Vector2<T> operator/=(Vector2<T> right);
    constexpr Vector2<T> operator/=(T right);
    constexpr Vector2<T> operator+=(Vector2<T> right);
    constexpr Vector2<T> operator-=(Vector2<T> right);
    constexpr bool operator==(Vector2<T> right);
    constexpr bool operator!=(Vector2<T> right);


    constexpr glm::vec2 ToGLMVec2();
    // constexpr Vector2<T> operator=(ImVec2 vec);
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
template<typename T>
constexpr bool operator<(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr bool operator>(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr bool operator<=(Vector2<T> left, Vector2<T> right);
template<typename T>
constexpr bool operator>=(Vector2<T> left, Vector2<T> right);

#include "BeatEngine/Graphics/Vector2.inl"
