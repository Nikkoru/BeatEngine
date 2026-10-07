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
    DrawCommand cmd{
        .projection = glm::mat4{ 0 },
        .transform = GetTransform().GetMatrix(),
        .padding = glm::vec2{ 0 },
        .textureID = m_Texture ? m_Texture.Get()->GetID() : Texture::NULL_ID,
        .shaderID = 0
    };

    state._DrawCommand = std::make_shared<DrawCommand>(cmd);
    state.DrawCommandSize = sizeof(DrawCommand);

    mgr->DrawVertices(m_Vertices, state);
}
