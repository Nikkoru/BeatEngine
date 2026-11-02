#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_core.h>
#include <shaderc/shaderc.hpp>

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Graphics/RenderState.hpp"
#include "BeatEngine/Renderers/Vulkan/Allocator.hpp"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"

class VulkanShaderData : public ShaderData {
public:
    std::vector<uint32_t> Spriv;
    std::vector<char> Buffer;
    size_t Size{};
};

class VulkanRenderer;
class VulkanShader : public Shader {
private:
    friend class VulkanRenderer;
private:
    std::function<GPUBuffer(size_t, VkBufferUsageFlags)> m_CreateBufFunc{}; 
    std::function<void(size_t, const VkWriteDescriptorSet&)> m_UpdateDescFunc{};
    std::function<void(const VkDescriptorSetLayoutCreateInfo&, VkDescriptorSetLayout*)> m_CreateDescFunc{}; 
    std::function<void(const VkDescriptorPoolCreateInfo&, VkDescriptorPool*)> m_CreatePoolFunc{}; 
    std::function<void(const VkDescriptorSetAllocateInfo&, VkDescriptorSet*)> m_AllocSetFunc{}; 

    VkShaderEXT m_ShaderVertImpl{ VK_NULL_HANDLE };
    VkShaderEXT m_ShaderFragImpl{ VK_NULL_HANDLE };
    VkShaderEXT m_ShaderCompImpl{ VK_NULL_HANDLE };

    VkShaderStageFlags m_Stages;

    std::vector<VkPushConstantRange> m_PushConstantRanges;
    std::map<std::string, std::vector<VkDescriptorSetLayout>> m_DescLayouts;
    VkDescriptorPoolSize m_PoolSize;
    VkDescriptorPool m_Pool;
    VkDescriptorSet m_Set;

    std::unordered_map<std::string, GPUBuffer> m_Buffers;
    VkPipelineLayout m_Layout{};

    std::unique_ptr<PushConstants> m_PushConstants{ nullptr };
    size_t m_PushConstantsSize{};
    
    std::string m_Name{};
public:
    VulkanShader() = default;
    ~VulkanShader() override = default;
public:
    bool GetFileContents(const std::filesystem::path& path, VulkanShaderData* data);
    bool Compile(VkDevice device, const std::vector<std::filesystem::path>& paths);

    bool LoadFromCompiledFile(VkDevice device, const std::vector<std::filesystem::path>& paths);

    VkShaderEXT const* GetShaderVertImpl();
    VkShaderEXT const* GetShaderFragImpl();
    VkShaderEXT const* GetShaderCompImpl();

    void Bind(VkCommandBuffer cmd);

    void SetStage(VkShaderStageFlagBits stage);
    void SetNextStage(VkShaderStageFlags stage);
    void SetPushConstants(VkPushConstantRange pushConstant) { m_PushConstantRanges.emplace_back(pushConstant); }

    void AddPushConstantRange(VkPushConstantRange range) { m_PushConstantRanges.emplace_back(range); }
    void AddDescLayout(VkDescriptorSetLayout layout) { m_DescLayouts[""].emplace_back(layout); }

    template<typename PushConstantT, typename... Args>
        requires(std::is_base_of_v<PushConstants, PushConstantT>)
    void PushConstant(Args&&... params) { m_PushConstants = std::make_unique<PushConstantT>(std::forward<Args>(params)...); m_PushConstantsSize = sizeof(PushConstantT); }

    GPUBuffer& GetBuffer(const std::string& name) { return m_Buffers.at(name); }
    VkPipelineLayout GetLayout() const { return m_Layout; }
    
    VkShaderStageFlags GetStages() const { return m_Stages; }
protected:
    void NewSSBO() override;
private:
    void _CreateShader(VkDevice device, VkShaderEXT* shader, VulkanShaderData* data, VkShaderStageFlagBits stage, VkShaderStageFlags nextStage);
    void _CreateLayout(VkDevice device);
};
