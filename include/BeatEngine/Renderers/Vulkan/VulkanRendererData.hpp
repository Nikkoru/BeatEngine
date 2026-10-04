#pragma once

#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"

class VulkanRenderer;
class VulkanRendererData : public RendererData {
private:
    friend class VulkanRenderer;

    GPUBuffer m_VertexBuffer{};
    GPUBuffer m_DrawCommandBuffer{};
public:
    VulkanRendererData() = default;
    ~VulkanRendererData() override = default;

};
