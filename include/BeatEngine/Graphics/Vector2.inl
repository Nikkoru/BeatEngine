#include "BeatEngine/Graphics/Vector2.h"

template<typename T>
constexpr Vector2<T>::Vector2(T x, T y) : X(x), Y(y) {
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator*=(Vector2<T> right) {
    this->X *= right.X;
    this->Y *= right.Y;
    return this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator*=(T right) {
    this->X *= right;
    this->Y *= right;
    return this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator/=(Vector2<T> right) {
    this->X /= right.X;
    this->Y /= right.Y;
    return this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator/=(T right) {
    this->X /= right;
    this->Y /= right;
    return this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator+=(Vector2<T> right) {
    this->X += right.X;
    this->Y += right.Y;
    return this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator-=(Vector2<T> right) {
    this->X -= right.X;
    this->Y -= right.Y;
    return this;
}

template<typename T>
constexpr bool Vector2<T>::operator==(Vector2<T> right) {
    return this->X == right.X && this->Y == right.Y;
}

template<typename T>
constexpr bool Vector2<T>::operator!=(Vector2<T> right) {
    return this->X != right.X || this->X != right.Y;
}

template<typename T>
constexpr Vector2<T> operator-(Vector2<T> left, Vector2<T> right) {
    return Vector2<T>(left.X - right.X, left.Y - right.Y);
}

template<typename T>
constexpr Vector2<T> operator+(Vector2<T> left, Vector2<T> right) {
    return Vector2<T>(left.X + right.X, left.Y + right.Y);
}

template<typename T>
constexpr Vector2<T> operator*(Vector2<T> left, Vector2<T> right) {
    return Vector2<T>(left.X * right.X, left.Y * right.Y);
}

template<typename T>
constexpr Vector2<T> operator*(Vector2<T> left, T right) {
    return Vector2<T>(left.X * right, left.Y * right);
}
template<typename T>
constexpr Vector2<T> operator*(T left, Vector2<T> right) {
    return Vector2<T>(left * right.X, left * right.Y);
}

template<typename T>
constexpr Vector2<T> operator/(Vector2<T> left, Vector2<T> right) {
    return Vector2<T>(left.X / right.X, left.Y / right.Y);
}

template<typename T>
constexpr Vector2<T> operator/(Vector2<T> left, T right) {
    return Vector2<T>(left.X / right.X, left.Y / right.Y);
}

template<typename T>
constexpr Vector2<T> operator/(T left, Vector2<T> right) {
    return Vector2<T>(left.X / right.X, left.Y / right.Y);
}
