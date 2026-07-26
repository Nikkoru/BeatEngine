#pragma once

#include "BeatEngine/Graphics/Rect.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include <glm/ext/matrix_float4x4.hpp>
class Angle;
class Transform {
private:
    glm::mat4 m_Matrix{ 1.f };
public:
    static const Transform Identity;
public:
    constexpr Transform() = default;

    constexpr Transform(glm::mat3 matrix3x3);

    constexpr const glm::mat4& GetMatrix() const;
    constexpr Transform GetInverse() const;

    constexpr Vector2f TransformPoint(const Vector2f& point) const;
    constexpr FloatRect TransformRect(const FloatRect& rect) const;

    constexpr Transform& Combine(const Transform& transform);
    constexpr Transform& Translate(Vector2f offset);

    Transform& Rotate(Angle angle);
    Transform& Rotate(Angle angle, Vector2f center);
    
    constexpr Transform& Scale(Vector2f factors);
    constexpr Transform& Scale(Vector2f factors, Vector2f center);

    constexpr Transform operator*(const Transform& other);
    constexpr Vector2f operator*(const Vector2f& other);

    constexpr Transform& operator*=(const Transform& other);

    constexpr bool operator==(const Transform& other);
    constexpr bool operator!=(const Transform& other) { return !(*this == other); }
};

#include "BeatEngine/Graphics/Transform.inl"
