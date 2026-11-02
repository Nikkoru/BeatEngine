#include "BeatEngine/Graphics/Shape.hpp"

#include <glm/ext/vector_float3.hpp>

#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Logger.h"

void Shape::UpdateVertex() {
    const auto vertexCount = GetPointCount();

    if (vertexCount < 3) {
        m_Vertices.Clear();
        Logger::AddWarning("", "Shape requires more than 2 vertices!");
        return;
    }

    m_Vertices.Resize(vertexCount);

    for (size_t i = 0; i < vertexCount; i++) {
        m_Vertices[i].Position = GetPointPosition(i);
        m_Vertices[i].Color = GetPointColor(i);
        m_Vertices[i].UV = GetUV(i);
    }

    m_Vertices.SetType(GetPrimitiveType());
}

void Shape::Draw(Renderer* const mgr, RenderState state) {
    state.transform = GetTransform();
    mgr->DrawVertices(m_Vertices, state);

    if (m_Texture)
        mgr->BindTextureToObject(m_Texture.Get().get());
}
