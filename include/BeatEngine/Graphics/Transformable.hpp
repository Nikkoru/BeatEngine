#pragma once

#include "BeatEngine/Graphics/Transform.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/System/Angle.hpp"
class Transformable {
private:
    Vector2f m_Origin{};
    Vector2f m_Position{};
    Angle m_Rotation{};
    Vector2f m_Scale{ 1, 1 };
    mutable Transform m_Transform{};
    mutable Transform m_InverseTransform{};
    mutable bool m_UpdateTransform{ true };
    mutable bool m_UpdateInverseTransform{ true };
public:
    Transformable() = default;
    virtual ~Transformable() = default;

    void SetPosition(Vector2f position);
    void SetRotation(Angle angle);
    void SetScale(Vector2f factors);
    void SetOrigin(Vector2f origin);

    Vector2f GetPosition() const { return m_Position; }
    Angle GetRotation() const { return m_Rotation; }
    Vector2f GetScale() const { return m_Scale; }
    Vector2f GetOrigin() const { return m_Origin; }

    void Move(Vector2f offset) { SetPosition(m_Position + offset); };
    void Rotate(Angle angle) { SetRotation(m_Rotation + angle); };
    void Scale(Vector2f factor) { SetScale(m_Scale + factor); };

    const Transform& GetTransform() const;
    const Transform& GetInverseTransform() const;
};
