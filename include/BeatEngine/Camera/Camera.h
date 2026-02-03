#pragma once

#include "BeatEngine/Camera/CameraMode.h"
#include "BeatEngine/Camera/ShakeParams.h"
#include "BeatEngine/Base/Entity.h"
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
class Camera {
private:
    std::shared_ptr<Base::Entity> m_Entity = nullptr;
    ShakeParams m_Shake;
    CameraMode m_Mode;
    float m_Zoom;
    sf::View m_View;
    bool m_AutoResizeToWindowSize = true;
public:
    Camera() = default;
    Camera(ShakeParams& shake) : m_Shake(shake) {}
    Camera(ShakeParams& shake, CameraMode& mode) : m_Shake(shake), m_Mode(mode) {}
public:
    void SetZoom(float zoom);
    void SetFollowingEntity(std::shared_ptr<Base::Entity> entity);
    void SetMode(CameraMode mode);
    void SetShakeParams(ShakeParams params);
    void SetPosition(sf::Vector2f pos);
    void SetSize(sf::Vector2f size);
    void SetAutoResize(bool resize);
public:
    float GetZoom() const;
    std::shared_ptr<Base::Entity> GetEntity() const;
    CameraMode GetMode() const;
    ShakeParams GetShakeParams() const;
    sf::Vector2f GetPosition() const;
    sf::Vector2f GetSize() const;
    bool IsAutoResize() const;
};
