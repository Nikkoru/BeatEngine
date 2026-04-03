#pragma once

#include "BeatEngine/Asset/Shader.h"
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanShader : public Shader {
private:
    VkShaderModule m_ShaderModule{ VK_NULL_HANDLE };
public:
    ~VulkanShader() override = default;
public:
    bool GetFileContents(const std::filesystem::path path) override;
    bool Compile(VkDevice device);

    VkShaderModule GetModule();
};

class VulkanShaderData : public ShaderData {
public:
    std::vector<uint32_t> Buffer;
    size_t Size{};
};
