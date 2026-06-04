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
    
    std::string m_Name{};
public:
    ~VulkanShader() override = default;
public:
    bool GetFileContents(const std::filesystem::path path) override;
    bool Compile(VkDevice device, std::filesystem::path path);

    VkShaderEXT GetShaderImpl();
};

class VulkanShaderData : public ShaderData {
public:
    std::vector<uint32_t> Spriv;
    std::vector<char> Buffer;
    size_t Size{};
};
