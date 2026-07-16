#include "BeatEngine/Renderers/Vulkan/BindlessSetManager.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "backends/imgui_impl_vulkan.h"
#include <array>
#include <vulkan/vulkan_core.h>

namespace {
static constexpr uint32_t MAX_BINDLESS_RESOURCES = 16536;
static constexpr uint32_t MAX_SAMPLERS = 32;

static constexpr uint32_t TEXTURES_BINDING = 0;
static constexpr uint32_t SAMPLERS_BINDING = 1;
}

void BindlessSetManager::Init(VkDevice device, float MaxAnisotropy) {
    {
        const auto poolSizesBindless = std::array<VkDescriptorPoolSize, 2>{{
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_BINDLESS_RESOURCES },
            { VK_DESCRIPTOR_TYPE_SAMPLER, MAX_SAMPLERS }        
        }};

        const auto poolInfo = VkDescriptorPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = MAX_BINDLESS_RESOURCES * poolSizesBindless.size(),
            .poolSizeCount = poolSizesBindless.size(),
            .pPoolSizes = poolSizesBindless.data()
        };

        VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescPool));
    }

    {
        const auto bindings = std::array<VkDescriptorSetLayoutBinding, 2>{{
            VkDescriptorSetLayoutBinding{ 
                .binding = TEXTURES_BINDING,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = MAX_BINDLESS_RESOURCES,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers{}
            },
            VkDescriptorSetLayoutBinding{ 
                .binding = SAMPLERS_BINDING,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = MAX_SAMPLERS,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers{}
            }
        }};

        const VkDescriptorBindingFlags bindlessFlags = 
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        const auto bindingFlags = std::array{ bindlessFlags, bindlessFlags };

        const auto flagInfo = VkDescriptorSetLayoutBindingFlagsCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = bindingFlags.size(),
            .pBindingFlags = bindingFlags.data()
        };
        const auto info = VkDescriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flagInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = bindings.size(),
            .pBindings = bindings.data()
        };

        VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &m_DescLayout));
    }

    {
        const auto allocInfo = VkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_DescPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_DescLayout
        };

        auto maxBinding = MAX_BINDLESS_RESOURCES - 1;
        const auto countInfo = VkDescriptorSetVariableDescriptorCountAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &maxBinding
        };

        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &m_DescSet));
    }

    InitDefaultSamplers(device, MaxAnisotropy);
}

void BindlessSetManager::Uninit(VkDevice device) {
    vkDestroySampler(device, m_NearestSampler, nullptr);
    vkDestroySampler(device, m_LinearSampler, nullptr);
    vkDestroySampler(device, m_ShadowMapSampler, nullptr);
    
    vkDestroyDescriptorSetLayout(device, m_DescLayout, nullptr);
    vkDestroyDescriptorPool(device, m_DescPool, nullptr);
}

void BindlessSetManager::InitDefaultSamplers(VkDevice device, float MaxAnisotropy) {
    static const uint32_t nearestSamplerId = 0;     
    static const uint32_t linearSamplerId = 1;     
    static const uint32_t shadowSamplerId = 2;     

    {
        const auto samplerCreateInfo = VkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = MaxAnisotropy
        };

        VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_NearestSampler));
        AddSampler(device, nearestSamplerId, m_NearestSampler);
    }
    
    {
        const auto samplerCreateInfo = VkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
        };

        VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_LinearSampler));
        AddSampler(device, linearSamplerId, m_LinearSampler);
    }

    {
        const auto samplerCreateInfo = VkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .compareEnable = VK_TRUE,
            .compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL
        };

        VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_ShadowMapSampler));
        AddSampler(device, shadowSamplerId, m_ShadowMapSampler);
    }
}

void BindlessSetManager::AddImage(VkDevice device, uint32_t id, VkImageView imageView) {
    const auto imageInfo = VkDescriptorImageInfo{
        .sampler{},
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    const auto writeSet = VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = m_DescSet,
        .dstBinding = TEXTURES_BINDING,
        .dstArrayElement = id,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo
    };
    vkUpdateDescriptorSets(device, 1, &writeSet, 0, nullptr);
}

void BindlessSetManager::AddSampler(VkDevice device, uint32_t id, VkSampler sampler) {
    const auto imageInfo = VkDescriptorImageInfo{
        .sampler = sampler,
        .imageView{},
        .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    };
    const auto writeSet = VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = m_DescSet,
        .dstBinding = SAMPLERS_BINDING,
        .dstArrayElement = id,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .pImageInfo = &imageInfo
    };
    vkUpdateDescriptorSets(device, 1, &writeSet, 0, nullptr);
}
