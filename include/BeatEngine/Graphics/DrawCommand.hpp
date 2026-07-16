#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class DrawCommand {
public:
    glm::mat4 transform;
    glm::vec2 padding;
    uint32_t textureID;
    uint32_t shaderID;
};
