#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>

#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/Vector2.h"

// struct GLSLVertex {
//     alignas(16) glm::vec3 position{};
//     alignas(16) glm::vec2 texCoords{};
//     alignas(16) LinearColor color{};
// };

struct Vertex {
    alignas(16) Vector2f Position{};
    alignas(16) Vector2f TexCoords{};
    alignas(16) LinearColor Color{};
};
