#pragma once

#include "BeatEngine/Graphics/Vector2.h"
template <typename T>
class Rect {
public:
    Vector2<T> Position{};
    Vector2<T> Size{};
public:
    constexpr Rect() = default;
    constexpr Rect(Vector2<T> position, Vector2<T> size)
        : Position(position), Size(size) {}

    constexpr Vector2<T> GetCenter() const { return Position + Size / T{2}; }
    constexpr bool operator==(Rect<T>& other) { return Position == other.Position && Size == other.Size; }

    template<typename U>
    constexpr explicit operator Rect<U>() const { return Rect<U>(Vector2<U>(Position), Vector2<U>(Size)); }
};

using IntRect = Rect<int>;
using FloatRect = Rect<float>;
