#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Logger.h"

template<typename... Args>
inline void AddVulkanLog(std::string fmt, Args&&... elms) {
#ifndef _WIN32
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", fmt, elms...);
#else
    Logger::AddLog("\x1b[0;41mVulkan\033[0m", "", fmt, elms...);
#endif
}

namespace VK {
struct Core {
    VkInstance Instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT DebugMessenger{ VK_NULL_HANDLE };
    VkPhysicalDeviceProperties DeviceProperties{};
    std::vector<VkExtensionProperties> DeviceExtensions{};
    VkImageFormatProperties ImageProperties{};
    VkPhysicalDevice PhysicalDevice{ VK_NULL_HANDLE };
    VkDevice Device{ VK_NULL_HANDLE };
    VkSurfaceKHR Surface{ VK_NULL_HANDLE };
    VmaAllocator Allocator{ VK_NULL_HANDLE };

    VkQueue GraphicsQueue{ VK_NULL_HANDLE };
    uint32_t GraphicsQueueFamily{};
};
}
