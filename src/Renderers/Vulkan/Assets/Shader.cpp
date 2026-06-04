#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"

#include "BeatEngine/Logger.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
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

    if (!f.is_open()) {
        Logger::AddError("", "Failed to create shader: cannot open file \"{}\"", path.string());
        return false;
    }

    auto data = std::make_shared<VulkanShaderData>();

    size_t fileSize = static_cast<size_t>(f.tellg());
    data->Buffer.resize(fileSize / sizeof(uint32_t));

    f.seekg(0);
    f.read(data->Buffer.data(), fileSize);
    f.close();

    m_Data = data;

    return true;
}

bool VulkanShader::Compile(VkDevice device, std::filesystem::path path) {
    if (!GetFileContents(path))
        return false;

    auto data = std::static_pointer_cast<VulkanShaderData>(m_Data);
    shaderc_shader_kind kind{};
    
    switch (m_Type) {
    case Type::Compute:
        kind = shaderc_compute_shader;
        m_Stage = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    case Type::Fragment:
        kind = shaderc_fragment_shader;
        m_Stage = VK_SHADER_STAGE_FRAGMENT_BIT;
       break;
    case Type::Vertex:
        kind = shaderc_vertex_shader;
        m_Stage = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    }
    
    shaderc::Compiler compiler{};
    shaderc::CompileOptions opts{};

    opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    opts.SetTargetSpirv(shaderc_spirv_version_1_6);
    opts.SetOptimizationLevel(shaderc_optimization_level_performance);
    
    shaderc::CompilationResult result = compiler.CompileGlslToSpv(data->Buffer.data(), kind, path.filename().c_str(), opts);
    
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        Logger::AddError("Shader Compilation Error: {}", result.GetErrorMessage());
        return false;
    }

    data->Spriv = { result.cbegin(), result.cend() };
    data->Size = data->Spriv.size() * sizeof(uint32_t);

    VkShaderCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags{},
        .stage = m_Stage,
        .nextStage = m_NextStage,
        .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
        .codeSize = data->Size,
        .pCode = data->Spriv.data(),
        .pName = "main",
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
        .pSpecializationInfo = nullptr
    };



    if (auto status = vkCreateShadersEXT(device, 1, &createInfo, nullptr, &m_ShaderImpl); status != VK_SUCCESS) {
        Logger::AddError("", "Failed to create shader: {}", string_VkResult(status));
        return false;
    }

    return true;
}

VkShaderEXT VulkanShader::GetShaderImpl() {
    return m_ShaderImpl;
}
