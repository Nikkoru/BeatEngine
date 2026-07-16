#include "BeatEngine/Graphics/Shape.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <glm/ext/vector_float3.hpp>

void Shape::Draw(GraphicsManager& mgr) {
    BaseDraw(mgr);
}

void Shape::UpdateVertex() {
    const auto vertexCount = GetPointCount();

    if (vertexCount < 3) {
        m_Vertices.clear();
        Logger::AddWarning("Shape requires more than 2 vertices!");
        return;
    }

    m_Vertices.resize(vertexCount);

    for (size_t i = 0; i < vertexCount; i++) {
        m_Vertices[i].Position = GetPointPosition(i);
        m_Vertices[i].Color = GetPointColor();
    }
}
