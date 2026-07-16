#pragma once

#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"
#include "BeatEngine/Renderers/Vulkan/Instance.h"
#include <vulkan/vulkan_core.h>

class GraphicsManager;
class GraphicalElement;
namespace VK {
class Instance;
}
class VulkanRendererData : public RendererData {
private:
    VkPipelineLayout m_PipelineLayout{};
    
    struct PushConstants {
        VkDeviceAddress CommandBuffer;
    };
    struct PerFrameData {
        GPUBuffer SpriteDrawCommandBuffer;
        GPUBuffer VertexBuffer;
    };

    std::array<PerFrameData, FRAME_OVERLAP> m_FramesData;
public:
    VulkanRendererData() = default;
    ~VulkanRendererData() override = default;

    PerFrameData& GetFrameData(uint32_t index) { return m_FramesData[index]; }
    
    void Init(VK::Instance& instance, VkFormat drawImageFormat);
    void Uninit(VK::Instance& instance);

    void DrawImGuiDrawData() override;

    void SetShader(std::filesystem::path& path, Shader::Type type) override;
    void PrepareDrawing() override {};
    void SetCommand(std::shared_ptr<DrawCommand> cmd) override;
    void MakeDraw(VK::Instance& instance, AllocatedImage& drawImage, VkCommandBuffer cmd);
};
