#pragma once

#include "BeatEngine/Camera/CameraMode.h"
#include "BeatEngine/Camera/ShakeParams.h"
#include "BeatEngine/Base/Entity.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>
class Camera {
private:
    glm::mat4 m_Projection{};
    float m_ZNear{};
    float m_ZFar{};

    bool m_Orthographic{ false };
    bool m_Orthographic2D{ false };
    bool m_UpdateProjection{ false };

    float m_AspectRatio{ 16.f / 9.f };

    Vector2f m_ViewSize{};

    std::shared_ptr<Base::Entity> m_Entity{ nullptr };
    ShakeParams m_Shake;
    CameraMode m_Mode;
    float m_Zoom;
    bool m_AutoResizeToWindowSize{ true };
    bool m_ClipSpaceYDown{ true };
public:
    Camera() = default;
    Camera(ShakeParams& shake) : m_Shake(shake) {}
    Camera(ShakeParams& shake, CameraMode& mode) : m_Shake(shake), m_Mode(mode) {}
public:
    void InitOrtho2D(const Vector2f& size, float zNear = .0f, float zFar = 1.f);
public:
    void SetZoom(float zoom) { m_Zoom = zoom; };
    void SetFollowingEntity(std::shared_ptr<Base::Entity> entity);
    void SetMode(CameraMode mode) { m_Mode = mode; };
    void SetShakeParams(ShakeParams params) { m_Shake = params; };
    void SetPosition(Vector2f pos) { (void)pos; };
    void SetSize(Vector2f size) { (void)size; };
    void SetAutoResize(bool resize) { m_AutoResizeToWindowSize = resize; };
public:
    float GetZoom() const;
    glm::mat4 GetProjection() const { return m_Projection; }
    std::shared_ptr<Base::Entity> GetEntity() const;
    CameraMode GetMode() const;
    ShakeParams GetShakeParams() const;
    Vector2f GetPosition() const;
    Vector2f GetSize() const;
    bool IsAutoResize() const;
};
