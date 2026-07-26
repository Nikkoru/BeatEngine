#include "BeatEngine/Graphics/Shape.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <glm/ext/vector_float3.hpp>

void Shape::Draw(GraphicsManager& mgr, RenderState state) {
    BaseDraw(mgr, state);
}

void Shape::UpdateVertex() {
    const auto vertexCount = GetPointCount();

    if (vertexCount < 3) {
        m_Vertices.Clear();
        Logger::AddWarning("Shape requires more than 2 vertices!");
        return;
    }

    m_Vertices.Resize(vertexCount);

    for (size_t i = 0; i < vertexCount; i++) {
        m_Vertices[i].Position = GetPointPosition(i);
        m_Vertices[i].Color = GetPointColor(i);
        m_Vertices[i].UV = GetUV(i);
    }

    m_PrimitiveType = GetPrimitiveType();
}
