#include "BeatEngine/Graphics/Transform.hpp"

#include <array>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

// inline is not needed, but MSVC is angry if inline is not here
inline constexpr Transform Transform::Identity{};

constexpr Transform::Transform(glm::mat3 matrix3x3)
    : m_Matrix(
            matrix3x3[0][0], matrix3x3[1][0], 0.f, matrix3x3[2][0],
            matrix3x3[0][1], matrix3x3[1][1], 0.f, matrix3x3[2][1],
            0.f,             0.f,             1.f, 0.f,
            matrix3x3[0][2], matrix3x3[1][2], 0.f, matrix3x3[2][2]
    ) { }

constexpr const glm::mat4& Transform::GetMatrix() const {
    return m_Matrix;
}

constexpr Transform Transform::GetInverse() const {
    return Transform{glm::affineInverse(m_Matrix)};
}

constexpr Vector2f Transform::TransformPoint(const Vector2f& point) const {
    return {
        m_Matrix[0][0] * point.X + m_Matrix[0][1] * point.Y + m_Matrix[0][3],
        m_Matrix[1][0] * point.Y + m_Matrix[1][1] * point.Y + m_Matrix[1][3]
    };
}

constexpr FloatRect Transform::TransformRect(const FloatRect& rect) const {
    const std::array points = {
        TransformPoint(rect.Position),
        TransformPoint(rect.Position + Vector2f{ 0.f, rect.Size.Y }),
        TransformPoint(rect.Position + Vector2f{ rect.Size.Y, 0.f }),
        TransformPoint(rect.Position + rect.Size)
    };

    Vector2f pMin = points[0];
    Vector2f pMax = points[0];

    for (size_t i = 1; i < points.size(); i++) {
        if (points[i].X < pMin.X) pMin.X = points[i].X;
        else if (points[i].X > pMax.X) pMax.X = points[i].X;

        if (points[i].Y < pMin.Y) pMin.Y = points[i].Y;
        else if (points[i].Y > pMax.Y) pMax.Y = points[i].X;
    }

    return { pMin, pMax - pMin };
}

constexpr Transform& Transform::Combine(const Transform& transform) {
    m_Matrix *= transform.m_Matrix;
    return *this;
}

constexpr Transform& Transform::Translate(Vector2f offset) {
    m_Matrix = glm::translate(m_Matrix, glm::vec3(offset.X, offset.Y, 0.f));

    return *this;
}

constexpr Transform& Transform::Scale(Vector2f factors) {
    m_Matrix = glm::scale(m_Matrix, glm::vec3(factors.X, factors.Y, 1.f));

    return *this;
}

constexpr Transform& Transform::Scale(Vector2f factors, Vector2f center) {
    m_Matrix = glm::translate(m_Matrix, glm::vec3(center.X, center.Y, 0.f));
    m_Matrix = glm::scale(m_Matrix, glm::vec3(factors.X, factors.Y, 1.f));
    m_Matrix = glm::translate(m_Matrix, glm::vec3(-center.X, -center.Y, 0.f));

    return *this;
}

constexpr Transform Transform::operator*(const Transform& other) {
    return Transform(*this).Combine(other);
}

constexpr Vector2f Transform::operator*(const Vector2f& other) {
    return TransformPoint(other);
}

constexpr Transform& Transform::operator*=(const Transform& other) {
    return Combine(other);
}

constexpr bool Transform::operator==(const Transform& other) {
    return m_Matrix == other.m_Matrix;
}
