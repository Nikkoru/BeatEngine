#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>

#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/Vector2.h"

struct Vertex {
    alignas(16) Vector2f Position{};
    alignas(16) Vector2f UV{};
    alignas(16) LinearColor Color{};
};

inline bool operator==(Vertex& lhs, Vertex rhs) { return (lhs.Position == rhs.Position) && (lhs.Color == rhs.Color) && (lhs.UV == rhs.UV); }
inline bool operator!=(Vertex& lhs, Vertex rhs) { return !(lhs == rhs); }
