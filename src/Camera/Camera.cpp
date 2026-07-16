#include "BeatEngine/Camera/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

void Camera::InitOrtho2D(const Vector2f& size, float zNear, float zFar) {
    m_ClipSpaceYDown = true;
    m_Orthographic2D = true;

    m_ZNear = zNear;
    m_ZFar = zFar;
    m_AspectRatio = size.X / size.Y;
    
    m_Projection = glm::ortho(0.f, size.X, 0.f, size.Y, m_ZNear, m_ZFar);

    m_ViewSize = size;
}
