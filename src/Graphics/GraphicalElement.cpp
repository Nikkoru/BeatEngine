#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>

void GraphicalElement::BaseDraw(GraphicsManager& mgr) {
    if (!Data) {
        mgr.InitElement(*this);
    }
    else if (!Data->IsInitialized()) {
        mgr.InitElement(*this);
    }

    Data->SetVertices(m_Vertices);

    glm::mat4 transform{};
    if (auto camera = mgr.GetMainCamera()) {
        transform = camera->GetProjection();
    }
    
    Data->PrepareDrawing();

    auto textureID{ NULL_IMAGE_ID };


    if (auto vulkanTexture = std::dynamic_pointer_cast<VulkanTexture>(m_Texture)) {
        textureID = vulkanTexture->GetID();        
    }


    DrawCommand cmd{
        .transform = transform,
        .padding = m_Padding.ToGLMVec2(),
        .textureID = textureID,
        .shaderID = 0,
    };
    SetCommand(std::make_shared<DrawCommand>(cmd));

    mgr.DrawElement(*this);
}

void GraphicalElement::UninitGraphics(GraphicsManager& mgr) {
    mgr.UninitElement(*this);
}

void GraphicalElement::DrawWindowImGuiDrawData() {
    ImGui::Begin("Random render data"); 
    DrawImGuiDrawData(); 
    ImGui::End();
}

void GraphicalElement::BaseDrawImGuiDrawData() {
    if (!Data) return;

    Data->DrawImGuiDrawData();
}

void GraphicalElement::SetCommand(std::shared_ptr<DrawCommand> cmd) {
    Data->SetCommand(cmd);
}
