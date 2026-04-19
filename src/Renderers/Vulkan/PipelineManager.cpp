#include "BeatEngine/Renderers/Vulkan/PipelineManager.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include <memory>
#include <volk.h>
#include <vulkan/vulkan_core.h>

void PipelineManager::Init(VkDescriptorSetLayout imageLayoutDescriptor, VkDescriptorSet imageDescriptor) {
   m_ImageLayoutDescriptor = imageLayoutDescriptor;
   m_ImageDescriptor = imageDescriptor;
}

void PipelineManager::DestroyAll(VkDevice device) {
        vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
        vkDestroyPipeline(device, m_Pipeline, nullptr);
}

void PipelineManager::LoadShader(VkDevice device, std::shared_ptr<Shader> shader, VkShaderStageFlagBits stageFlags) {
    if (m_ImageDescriptor == VK_NULL_HANDLE) {
        Logger::AddError("\e[0;41mVulkan\033[0m", "ImageDescriptor not initialized. Cannot load shader");
        return;
    }
    if (m_ImageLayoutDescriptor == VK_NULL_HANDLE){
        Logger::AddError("\e[0;41mVulkan\033[0m", "ImageLayoutDescriptor not initialized. Cannot load shader");
        return;
    }

    auto vulkanShader = std::static_pointer_cast<VulkanShader>(shader);
    
    VkPipelineLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = 1,
        .pSetLayouts = &m_ImageLayoutDescriptor
    };

    VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_PipelineLayout));

    VkPipelineShaderStageCreateInfo stageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stageFlags,
        .module = vulkanShader->GetModule(),
        .pName = "main"
    };
    VkComputePipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .stage = stageInfo,
        .layout = m_PipelineLayout,
    };

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
}

bool PipelineManager::BindIfAny(VkCommandBuffer cmd) {
    if (m_ImageDescriptor == VK_NULL_HANDLE) {
        Logger::AddError("\e[0;41mVulkan\033[0m", "ImageDescriptor not initialized. Cannot bind pipeline");
        return false;
    }
    if (m_ImageLayoutDescriptor == VK_NULL_HANDLE){
        Logger::AddError("\e[0;41mVulkan\033[0m", "ImageLayoutDescriptor not initialized. Cannot bind pipeline");
        return false;
    }
    if (m_Pipeline == VK_NULL_HANDLE)
        return false;
    if (m_PipelineLayout == VK_NULL_HANDLE)
        return false;

    if (m_Pipeline != VK_NULL_HANDLE && m_PipelineLayout != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout, 0, 1, &m_ImageDescriptor, 0, nullptr);
        return true;
    }

    return false;
}

bool PipelineManager::Has() {
    return m_Pipeline != VK_NULL_HANDLE;
}
