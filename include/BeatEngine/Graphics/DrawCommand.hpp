#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

class DrawCommand {
public:
    glm::mat4 projection{ 0 };
    glm::mat4 transform{ 0 };
    glm::vec2 padding;
    uint32_t textureID;
    uint32_t shaderID;
};
