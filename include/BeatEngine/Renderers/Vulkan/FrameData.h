#pragma once

#include "BeatEngine/Renderers/Vulkan/UninitQueue.h"
#include "vulkan/vulkan_core.h"
struct FrameData {
    VkSemaphore PresentSemaphore{ VK_NULL_HANDLE }, RenderSemaphore{ VK_NULL_HANDLE };
    VkFence RenderFence{ VK_NULL_HANDLE };
    VkCommandBuffer ActiveCmdBuffer{ VK_NULL_HANDLE };
    VkCommandPool CommandPool{ VK_NULL_HANDLE };

    UninitQueue Uninitializer;
};

constexpr unsigned int FRAME_OVERLAP = 2;
