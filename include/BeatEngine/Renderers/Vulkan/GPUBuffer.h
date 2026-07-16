#pragma once

#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include <vulkan/vulkan_core.h>

struct GPUBuffer {
    size_t BufferSize{};
    VkBuffer Buffer{ VK_NULL_HANDLE };
    VmaAllocation Allocation{};
    VmaAllocationInfo AllocationInfo{};

    VkDeviceAddress Address{};
};
