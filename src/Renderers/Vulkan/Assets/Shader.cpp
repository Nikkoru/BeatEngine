#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"

#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Allocator.hpp"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include <cstdint>
#include <cstring>
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
#include <BeatEngine/System/String.hpp>

bool VulkanShader::GetFileContents(const std::filesystem::path& path, VulkanShaderData* data) {

    auto f = std::ifstream(path, std::ios::ate | std::ios::binary);

    if (!f.is_open()) {
        Logger::AddError("", "Failed to create shader: cannot open file \"{}\"", path.string());
        return false;
    }

    size_t fileSize = static_cast<size_t>(f.tellg());
    data->Buffer.resize(fileSize);

    f.seekg(0);
    f.read(data->Buffer.data(), fileSize);
    f.close();

    return true;
}

bool VulkanShader::Compile(VkDevice device, const std::vector<std::filesystem::path>& paths) {
    auto i = 0;
    for (const auto& path : paths) {
        if (i >= 3) break;
        if (!std::filesystem::exists(path)) {
            Logger::AddError("", "Failed to create shader: path is not valid (\"{}\")", path.string());
            return false;
        }

        auto ext = path.extension().string();
        auto curType = Shader::Type::Vertex;
        if (ext.contains("vert")) {
            curType = Shader::Type::Vertex;
        }
        else if (ext.contains("frag")) {
            curType = Shader::Type::Fragment;

        }
        else if (ext.contains("comp")) {
            curType = Shader::Type::Compute;
        }
        else {
            AddVulkanLog("Unknown type for file: \"{}\", Using default order. (1. Vertex, 2. Fragment, 3. Compute, current step: {})", path.string(), i + 1);
            if (i == 0)
                curType = Shader::Type::Vertex;
            else if (i == 1)
                curType = Shader::Type::Fragment;
            else if (i == 2)
                curType = Shader::Type::Compute;
            else {
                Logger::AddError(typeid(VulkanRenderer), "Failed to create shader: Couldn't find a suitable type for file: \"{}\"", path.string());
                return false;
            }
        }

        auto data = std::static_pointer_cast<VulkanShaderData>(m_Data.at(curType));
        shaderc_shader_kind kind{};
        VkShaderEXT* shader = nullptr;
        VkShaderStageFlagBits stage{};
        VkShaderStageFlags nextStage{};

        switch (curType) {
        case Shader::Type::Vertex:
            kind = shaderc_vertex_shader;
            shader = &m_ShaderVertImpl;
            stage = VK_SHADER_STAGE_VERTEX_BIT;
            nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case Shader::Type::Fragment:
            kind = shaderc_fragment_shader;
            shader = &m_ShaderFragImpl;
            stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            nextStage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        case Shader::Type::Compute:
            kind = shaderc_compute_shader;
            shader = &m_ShaderFragImpl;
            stage = VK_SHADER_STAGE_COMPUTE_BIT;
            nextStage = 0;
            break;
        }

        if (!GetFileContents(path, data.get()))
            return false;

        shaderc::Compiler compiler{};
        shaderc::CompileOptions opts{};

        opts.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
        opts.SetTargetSpirv(shaderc_spirv_version_1_6);
        opts.SetOptimizationLevel(shaderc_optimization_level_performance);

        String filenameStr = path.filename().wstring();

        shaderc::CompilationResult result = compiler.CompileGlslToSpv(data->Buffer.data(), kind, filenameStr.ToCString(true), opts);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        Logger::AddError("", "Shader Compilation Error: {}", result.GetErrorMessage());
        return false;
        }

        data->Spriv = { result.cbegin(), result.cend() };
        data->Size = data->Spriv.size() * sizeof(uint32_t);

        _CreateShader(device, shader, data.get(), stage, nextStage);
    }

    return true;
}

bool VulkanShader::LoadFromCompiledFile(VkDevice device, const std::vector<std::filesystem::path>& paths) {
    if (m_Data.empty()) {
        m_Data[Type::Vertex] = std::make_shared<VulkanShaderData>();
        m_Data[Type::Fragment] = std::make_shared<VulkanShaderData>();
        m_Data[Type::Compute] = std::make_shared<VulkanShaderData>();
    }

    std::unordered_map<Shader::Type, std::filesystem::path> shaderMap;

    auto i = 0;
    for (const auto& path : paths) {
        if (i >= 3) break;
        if (!std::filesystem::exists(path)) {
            Logger::AddError("", "File {} doesn't exists", path.string());
            return false;
        }
        auto ext = path.extension().string();
        auto curType = Shader::Type::Vertex;
        if (ext.contains("vert") || !shaderMap.contains(Shader::Type::Vertex)) {
            curType = Shader::Type::Vertex;
        }
        else if (ext.contains("frag") || !shaderMap.contains(Shader::Type::Fragment)) {
            curType = Shader::Type::Fragment;

        }
        else if (ext.contains("comp") || !shaderMap.contains(Shader::Type::Compute)) {
            curType = Shader::Type::Compute;
        }
        else {
            AddVulkanLog("Unknown type for file: \"{}\", Using default order. (1. Vertex, 2. Fragment, 3. Compute, current step: {})", path.string(), i + 1);
            if (!shaderMap.contains(Shader::Type::Vertex))
                curType = Shader::Type::Vertex;
            else if (!shaderMap.contains(Shader::Type::Fragment))
                curType = Shader::Type::Fragment;
            else if (!shaderMap.contains(Shader::Type::Compute))
                curType = Shader::Type::Compute;
            else {
                Logger::AddError(typeid(VulkanRenderer), "Failed to craete shader: Couldn't find a suitable type for file: \"{}\"", path.string());
                return false;
            }
        }

        shaderMap[curType] = path;
        i++;
    }

    for (const auto& [type, path] : shaderMap) {
        auto curData = m_Data.at(type);

        std::ifstream file{ path, std::ios::ate | std::ios::binary };

        if (!file.is_open()) {
            Logger::AddError("", "Failed to open {}", path.string());
            return false;
        }

        const auto fileSize = file.tellg();
        auto data = std::static_pointer_cast<VulkanShaderData>(curData);
        data->Spriv.resize(fileSize / sizeof(uint32_t));
        data->Size = fileSize /* * sizeof(uint32_t) */;

        file.seekg(0);
        file.read((char*)data->Spriv.data(), fileSize);
        file.close();



        VkShaderEXT* shader = nullptr;
        VkShaderStageFlagBits stage{};
        VkShaderStageFlags nextStage{};

        switch (type) {
        case Shader::Type::Vertex:
            shader = &m_ShaderVertImpl;
            stage = VK_SHADER_STAGE_VERTEX_BIT;
            nextStage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case Shader::Type::Fragment:
            shader = &m_ShaderFragImpl;
            stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            nextStage = 0;
            break;
        case Shader::Type::Compute:
            shader = &m_ShaderCompImpl;
            stage = VK_SHADER_STAGE_COMPUTE_BIT;
            nextStage = 0;
            break;
        }
        _CreateShader(device, shader, data.get(), stage, nextStage);
        m_Stages |= stage;
    }

    _CreateLayout(device);
    return true;
}

void VulkanShader::Destroy(VkDevice device, VK::Allocator& alloc) {
    vkDestroyPipelineLayout(device, m_Layout, nullptr);
    vkDestroyDescriptorPool(device, m_Pool, nullptr);

    if (!m_Buffers.empty()) {
        for (auto& [_, buf] : m_Buffers) {
            alloc.DestroyBuffer(buf); 
        }
    }

    if (m_ShaderVertImpl)
        vkDestroyShaderEXT(device, m_ShaderVertImpl, nullptr);
    if (m_ShaderFragImpl)
        vkDestroyShaderEXT(device, m_ShaderFragImpl, nullptr);
    if (m_ShaderCompImpl)
        vkDestroyShaderEXT(device, m_ShaderCompImpl, nullptr);
}

VkShaderEXT const* VulkanShader::GetShaderVertImpl() {
    return &m_ShaderVertImpl;
}

VkShaderEXT const* VulkanShader::GetShaderFragImpl() {
    return &m_ShaderFragImpl;
}

VkShaderEXT const* VulkanShader::GetShaderCompImpl() {
    return &m_ShaderCompImpl;
}

void VulkanShader::Bind(VkCommandBuffer cmd) {
    std::vector<VkShaderStageFlagBits> stages;
    VkShaderStageFlags pushStages{};
    std::vector<VkShaderEXT> shaders;
    if (m_ShaderVertImpl) {
        stages.emplace_back(VK_SHADER_STAGE_VERTEX_BIT);
        pushStages |= VK_SHADER_STAGE_VERTEX_BIT;
        shaders.emplace_back(m_ShaderVertImpl);
    }
    if (m_ShaderFragImpl) {
        stages.emplace_back(VK_SHADER_STAGE_FRAGMENT_BIT);
        pushStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        shaders.emplace_back(m_ShaderFragImpl);
    }
    if (m_ShaderCompImpl) {
        stages.emplace_back(VK_SHADER_STAGE_COMPUTE_BIT);
        pushStages |= VK_SHADER_STAGE_COMPUTE_BIT;
        shaders.emplace_back(m_ShaderVertImpl);
    }
    stages.emplace_back(VK_SHADER_STAGE_GEOMETRY_BIT);
    shaders.emplace_back(nullptr);
    stages.emplace_back(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    shaders.emplace_back(nullptr);
    stages.emplace_back(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    shaders.emplace_back(nullptr);

    if (m_PushConstants) {
        vkCmdPushConstants(
            cmd,
            m_Layout,
            pushStages,
            0,
            m_PushConstantsSize,
            m_PushConstants.get()
        );
    }

    if (m_Set != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Layout, 0, 1, &m_Set, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Layout, 0, 1, &m_Set, 0, nullptr);
    }

    vkCmdBindShadersEXT(cmd, shaders.size(), stages.data(), shaders.data());
}

void VulkanShader::NewSSBO() {
    if (!m_SSBOs.empty()) {
        for (auto& [name, ssbo] : m_SSBOs) {
            if (m_Buffers.contains(name)) continue;

            m_Buffers[name] = m_CreateBufFunc(ssbo.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

            memcpy(
                m_Buffers.at(name).AllocationInfo.pMappedData,
                ssbo.data,
                ssbo.size
            );

            if (ssbo.binding != static_cast<uint32_t>(-1)) {
                VkDescriptorSetLayoutBinding ssboLayoutBinding{};
                ssboLayoutBinding.binding = ssbo.binding;
                ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                ssboLayoutBinding.descriptorCount = 1;
                ssboLayoutBinding.stageFlags = GetStages();
                ssboLayoutBinding.pImmutableSamplers = nullptr;

                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = 1;
                layoutInfo.pBindings = &ssboLayoutBinding;

                VkDescriptorSetLayout descriptorSetLayout{};
                m_CreateDescFunc(layoutInfo, &descriptorSetLayout);
                m_DescLayouts["ssbo"].emplace_back(descriptorSetLayout);
                m_PoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                m_PoolSize.descriptorCount = m_DescLayouts.size();

                VkDescriptorPoolCreateInfo info{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                    .maxSets = static_cast<uint32_t>(m_DescLayouts.size()),
                    .poolSizeCount = 1,
                    .pPoolSizes = &m_PoolSize
                };

                m_CreatePoolFunc(info, &m_Pool);

                VkDescriptorSetAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .descriptorPool = m_Pool,
                    .descriptorSetCount = static_cast<uint32_t>(m_DescLayouts.at("ssbo").size()),
                    .pSetLayouts = m_DescLayouts.at("ssbo").data()
                };

                m_AllocSetFunc(allocInfo, &m_Set);

                for (const auto& [name, buf] : m_Buffers) {
                    if (m_SSBOs.at(name).binding == static_cast<uint32_t>(-1)) continue;
                    VkDescriptorBufferInfo info{
                        .buffer = buf.Buffer,
                        .offset = 0,
                        .range = VK_WHOLE_SIZE
                    };

                    VkWriteDescriptorSet descriptorWrite{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = m_Set,
                        .dstBinding = m_SSBOs.at(name).binding,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo{},
                        .pBufferInfo = &info,
                        .pTexelBufferView{}
                    };

                    m_UpdateDescFunc(1, descriptorWrite);
                }
            }
        }
    }
}

void VulkanShader::_CreateShader(VkDevice device, VkShaderEXT* shader, VulkanShaderData* data, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage) {
    std::vector<VkDescriptorSetLayout> shaderLayouts;
    for (const auto& [name, layouts] : m_DescLayouts)
        for (const auto& layout : layouts)
            shaderLayouts.emplace_back(layout);

    VkShaderCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
        .pNext = nullptr,
        // .flags = VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT,
        .flags{},
        .stage = stage,
        .nextStage = nextStage,
        .codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT,
        .codeSize = data->Size,
        .pCode = data->Spriv.data(),
        .pName = "main",
        .setLayoutCount = static_cast<uint32_t>(shaderLayouts.size()),
        .pSetLayouts = m_DescLayouts.empty() ? nullptr : shaderLayouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(m_PushConstantRanges.size()),
        .pPushConstantRanges = m_PushConstantRanges.empty() ? nullptr : m_PushConstantRanges.data(),
        .pSpecializationInfo = nullptr,
    };

    if (auto status = vkCreateShadersEXT(device, 1, &createInfo, nullptr, shader); status != VK_SUCCESS) {
        Logger::AddError(typeid(VulkanRenderer), "Failed to create shader: {}", string_VkResult(status));
    }
}

void VulkanShader::_CreateLayout(VkDevice device) {
    std::vector<VkDescriptorSetLayout> shaderLayouts;
    for (const auto& [name, layouts] : m_DescLayouts)
        for (const auto& layout : layouts)
            shaderLayouts.emplace_back(layout);

    m_Layout = vkb::CreatePipelineLayout(device, shaderLayouts, m_PushConstantRanges);
}
