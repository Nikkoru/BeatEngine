#include "BeatEngine/View/ViewLayer.h"
#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/Camera/CameraMode.h"

void ViewLayer::SetCamera(Camera& camera) {
    this->m_Camera = camera;
}

void ViewLayer::SetCameraMode(CameraMode mode) {
    this->m_Camera.SetMode(mode);
}

void ViewLayer::SetCameraPosition(Vector2f pos) {
    this->m_Camera.SetPosition(pos);
}

void ViewLayer::SetCameraRotation(float rotation) {
    return;
}

void ViewLayer::SetCameraZoom(float zoom) {
    this->m_Camera.SetZoom(zoom);
}

void ViewLayer::ShakeCamera(ShakeParams params) {
    this->m_Camera.SetShakeParams(params); 
}

Vector2f ViewLayer::GetScreenToWorld(Vector2f pos) const {
    return {};
}
Vector2f ViewLayer::GetWorldToScreen(Vector2f pos) const {
    return {};
}
void ViewLayer::StartCamera() {

}
void ViewLayer::StopCamera() {

}
