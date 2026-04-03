#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"

#include "BeatEngine/Logger.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <volk.h>

bool VulkanShader::GetFileContents(const std::filesystem::path path) {
    if (!std::filesystem::exists(path)) {
        Logger::AddError("", "Failed to create shader: path is not valid (\"{}\")", path.string());
        return false;
    }

    auto f = std::ifstream(path, std::ios::ate | std::ios::binary);
    auto data = std::make_shared<VulkanShaderData>();

    if (!f.is_open()) {
        Logger::AddError("", "Failed to create shader: cannot open file \"{}\"", path.string());
        return false;
    }

    size_t fileSize = static_cast<size_t>(f.tellg());
    data->Buffer.resize(fileSize / sizeof(uint32_t));
    data->Size = fileSize;

    f.seekg(0);
    f.read((char*)(data->Buffer.data()), fileSize);
    f.close();

    m_Data = std::move(data);

    return true;
}

bool VulkanShader::Compile(VkDevice device) {
    auto data = std::static_pointer_cast<VulkanShaderData>(m_Data);

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .codeSize = data->Size,
        .pCode = data->Buffer.data()
    };

    auto status = vkCreateShaderModule(device, &createInfo, nullptr, &m_ShaderModule);
    if (status != VK_SUCCESS) {
        Logger::AddError("", "Failed to create shader: {}", string_VkResult(status));
        return false;
    }

    return true;
}

VkShaderModule VulkanShader::GetModule() {
    return m_ShaderModule;
}
