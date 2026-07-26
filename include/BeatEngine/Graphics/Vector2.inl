#include "BeatEngine/Graphics/Vector2.h"

template<typename T>
constexpr Vector2<T>::Vector2(T x) : X(x), Y(x) {}

template<typename T>
constexpr Vector2<T>::Vector2(T x, T y) : X(x), Y(y) {
}

// template<typename T>
// constexpr Vector2<T>::Vector2(ImVec2 vec) : X(vec.x), Y(vec.y) {
// }

template<typename T>
constexpr Vector2<T> Vector2<T>::operator*=(Vector2<T> right) {
    this->X *= right.X;
    this->Y *= right.Y;
    return *this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator*=(T right) {
    this->X *= right;
    this->Y *= right;
    return *this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator/=(Vector2<T> right) {
    this->X /= right.X;
    this->Y /= right.Y;
    return *this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator/=(T right) {
    this->X /= right;
    this->Y /= right;
    return *this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator+=(Vector2<T> right) {
    this->X += right.X;
    this->Y += right.Y;
    return *this;
}

template<typename T>
constexpr Vector2<T> Vector2<T>::operator-=(Vector2<T> right) {
    this->X -= right.X;
    this->Y -= right.Y;
    return *this;
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
constexpr glm::vec2 Vector2<T>::ToGLMVec2() {
    return glm::vec2(this->X, this->Y);
}


template<typename T>
constexpr Vector2<T> Vector2<T>::ComponentWiseDivision(Vector2<T> other) {
    return { X / other.X, Y / other.Y };
}

template<typename T>
constexpr Vector2<T> Vector2<T>::ComponentWiseMultiplication(Vector2<T> other) {
    return { X * other.X, Y * other.Y };
}

// template<typename T>
// constexpr Vector2<T> Vector2<T>::operator=(ImVec2 vec) {
//     this->X = vec.x;
//     this->Y = vec.y;
//     return *this;
// }

// template<typename T>
// constexpr Vector2<T> operator+=(Vector2<T>& left, Vector2<T> right) {
//     left.X += right.X;
//     left.Y += right.Y;
//     return left;
// }

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
    return Vector2<T>(left.X / right, left.Y / right);
}

template<typename T>
constexpr Vector2<T> operator/(T left, Vector2<T> right) {
    return Vector2<T>(left.X / right.X, left.Y / right.Y);
}

template<typename T>
constexpr bool operator<(Vector2<T> left, Vector2<T> right) {
    return left.X < right.X && left.Y < right.Y;
}

template<typename T>
constexpr bool operator>(Vector2<T> left, Vector2<T> right) {
    return left.X > right.X && left.Y > right.Y;
}

template<typename T>
constexpr bool operator<=(Vector2<T> left, Vector2<T> right) {
    return left.X <= right.X && left.Y <= right.Y;
}

template<typename T>
constexpr bool operator>=(Vector2<T> left, Vector2<T> right) {
    return left.X >= right.X && left.Y >= right.Y;
}
