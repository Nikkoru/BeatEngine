#pragma once

#include "vulkan/vulkan_core.h"
struct FrameData {
    VkSemaphore  PresentSemaphore{};
    VkFence RenderFence{};
};

constexpr unsigned int FRAME_OVERLAP = 2;
