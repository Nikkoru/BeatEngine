#include "BeatEngine/Graphics/Transform.hpp"
#include "BeatEngine/System/Angle.hpp"

Transform& Transform::Rotate(Angle angle) {
    auto identity = glm::mat4{ 1.f };
    identity = glm::rotate(identity, angle.AsRadians(), glm::vec3(0.f, 1.f, 0.f));

    return Combine(Transform{ identity });
}

Transform& Transform::Rotate(Angle angle, Vector2f center) {
    auto identity = glm::mat4{ 1.f };
    identity = glm::translate(identity, glm::vec3(center.X, center.Y, 0.f));
    identity = glm::rotate(identity, angle.AsRadians(), glm::vec3(0.f, 1.f, 0.f));
    identity = glm::translate(identity, glm::vec3(-center.X, -center.Y, 0.f));

    return Combine(Transform{ identity });
    
}
