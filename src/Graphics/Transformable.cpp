#include "BeatEngine/Graphics/Transformable.hpp"

void Transformable::SetPosition(Vector2f position) {
    if (m_Position == position) return;

    m_Position = position;
    m_UpdateTransform = true;
    m_UpdateInverseTransform = true;
}

void Transformable::SetRotation(Angle angle) {
    if (m_Rotation == angle) return;

    m_Rotation = angle;
    m_UpdateTransform = true;
    m_UpdateInverseTransform = true;
}

void Transformable::SetScale(Vector2f factors) {
    if (m_Scale == factors) return;

    m_Scale = factors;
    m_UpdateTransform = true;
    m_UpdateInverseTransform = true;
}

void Transformable::SetOrigin(Vector2f origin) {
    if (m_Origin == origin) return;

    m_Origin = origin;
    m_UpdateTransform = true;
    m_UpdateInverseTransform = true;
}

const Transform& Transformable::GetTransform() const {
    if (!m_UpdateTransform) return m_Transform;

    m_Transform = Transform::Identity;

    m_Transform.Translate(m_Position);
    m_Transform.Rotate(m_Rotation);
    m_Transform.Scale(m_Scale);

    m_UpdateTransform = false;

    return m_Transform;
}

const Transform& Transformable::GetInverseTransform() const {
    if (!m_UpdateInverseTransform) return m_InverseTransform;

    m_InverseTransform = m_Transform.GetInverse();
    m_UpdateInverseTransform = false;

    return m_InverseTransform;
}
