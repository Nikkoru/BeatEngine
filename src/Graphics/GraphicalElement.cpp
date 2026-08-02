#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>

void GraphicalElement::BaseDraw(GraphicsManager& mgr, RenderState state) {
    glm::mat4 transform{ 1.f };
    if (auto camera = mgr.GetMainCamera()) {
        transform = camera->GetProjection();
    }

    DrawCommand cmd{
        .projection = transform,
        .transform = glm::mat4{ 1.f },
        .padding = m_Padding.ToGLMVec2(),
        .textureID = Texture::NULL_ID,
        .shaderID = 0,
    };
    if (m_Texture)
        cmd.textureID = m_Texture.Get()->GetID();

    state._DrawCommand = std::make_shared<DrawCommand>(cmd);
    state.DrawCommandSize = sizeof(DrawCommand);

    m_Vertices.SetType(m_PrimitiveType);

    mgr.DrawVertices(m_Vertices, state);
}

void GraphicalElement::UninitGraphics(GraphicsManager& mgr) {
    mgr.UninitElement(*this);
}

void GraphicalElement::SetTexture(Base::AssetHandle<Texture> texture) {
    m_Texture = texture;
}

void GraphicalElement::DrawWindowImGuiDrawData() {
    ImGui::Begin("Random render data"); 
    DrawImGuiDrawData(); 
    ImGui::End();
}

void GraphicalElement::BaseDrawImGuiDrawData() {
}

void GraphicalElement::SetCommand(std::shared_ptr<DrawCommand> cmd) {
    (void)cmd;
}
