#pragma once

#include "BeatEngine/Asset/Shader.h"
#include <vector>
#include <vulkan/vulkan_core.h>

class PipelineManager {
private:
    VkPipeline m_Pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

    VkDescriptorSetLayout m_ImageLayoutDescriptor{ VK_NULL_HANDLE };
    VkDescriptorSet m_ImageDescriptor{ VK_NULL_HANDLE };

    std::map<Shader::Type, std::vector<VkShaderModule>> m_Shaders;
public:
    PipelineManager() = default;
    ~PipelineManager() = default;
public:
    void Init(VkDescriptorSetLayout imageLayoutDescriptor, VkDescriptorSet imageDescriptor);
    void DestroyAll(VkDevice device);
    void LoadShader(VkDevice device, std::shared_ptr<Shader> shader, VkShaderStageFlagBits stageFlags);
    bool BindIfAny(VkCommandBuffer cmd);
    bool Has();
};
