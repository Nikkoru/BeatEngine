#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

struct QueueInfo {
    uint32_t familyIndex = ~0U;
    uint32_t queueIndex = ~0U;
    VkQueue queue{};
};
