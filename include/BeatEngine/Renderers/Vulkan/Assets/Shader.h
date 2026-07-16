#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>
#include <shaderc/shaderc.hpp>

#include "BeatEngine/Asset/Shader.h"

class VulkanShader : public Shader {
private:
    VkShaderEXT m_ShaderImpl{ VK_NULL_HANDLE };
    VkShaderStageFlagBits m_Stage{};
    VkShaderStageFlags m_NextStage{};

    std::vector<VkPushConstantRange> m_PushConstants;
    std::vector<VkDescriptorSetLayout> m_DescLayouts;
    
    std::string m_Name{};
    
    void* m_pNext = nullptr;
public:
    ~VulkanShader() override = default;
public:
    bool GetFileContents(const std::filesystem::path path) override;
    bool Compile(VkDevice device, std::filesystem::path path);

    bool LoadFromCompiledFile(VkDevice device, std::filesystem::path path);

    VkShaderEXT const* GetShaderImpl();

    void SetStage(VkShaderStageFlagBits stage);
    void SetPNext(void* next) { m_pNext = next; }
    void SetNextStage(VkShaderStageFlags stage);
    void AddPushConstants(VkPushConstantRange pushConstant) { m_PushConstants.emplace_back(pushConstant); }
    void AddDescLayout(VkDescriptorSetLayout layout) { m_DescLayouts.emplace_back(layout); }
    VkShaderStageFlagBits& GetStage() { return m_Stage; }
    VkShaderStageFlags& GetNextStage() { return m_NextStage; }
};

class VulkanShaderData : public ShaderData {
public:
    std::vector<uint32_t> Spriv;
    std::vector<char> Buffer;
    size_t Size{};
};
