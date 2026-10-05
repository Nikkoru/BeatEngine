#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>

void GraphicalElement::BaseDraw(Renderer* const mgr, RenderState state) {
    glm::mat4 proj{ 0.f };
    // if (auto camera = mgr.GetMainCamera()) {
    //     proj = camera->GetProjection();
    // }

    DrawCommand cmd{
        .projection = proj,
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

    mgr->DrawVertices(m_Vertices, state);
}

void GraphicalElement::UninitGraphics(Renderer* const mgr) {
    mgr->UninitVertices(m_Vertices);
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
