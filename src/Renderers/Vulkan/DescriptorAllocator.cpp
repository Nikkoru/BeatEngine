#include "BeatEngine/Renderers/Vulkan/DescriptorAllocator.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <volk.h>

void DescriptorAllocator::InitPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto ratio : poolRatios) {
        poolSizes.emplace_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = uint32_t(ratio.ratio * maxSets)
        });
    }

    VkDescriptorPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = maxSets,
        .poolSizeCount = (uint32_t)poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    vkCreateDescriptorPool(device, &info, nullptr, &Pool);
}

void DescriptorAllocator::ClearDescriptors(VkDevice device) {
    vkResetDescriptorPool(device, Pool, 0);
}

void DescriptorAllocator::DestroyPool(VkDevice device) {
    vkDestroyDescriptorPool(device, Pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::Allocate(VkDevice device, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = Pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout
    };

    VkDescriptorSet ds;
    VK_CHECK(vkAllocateDescriptorSets(device, &info, &ds));

    return ds;
}
