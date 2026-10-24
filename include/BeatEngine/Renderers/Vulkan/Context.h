#pragma once

#include "BeatEngine/Renderers/Vulkan/QueueInfo.hpp"
#include <vector>

#include <vulkan/vulkan_core.h>

class AppContext;
class VulkanRenderer;
namespace VK {
struct DeviceExtension {
    const char* name = nullptr;
    bool required = false;
    void* featureStruct = nullptr;
};
struct ContextCreateConfig {
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;
    std::vector<DeviceExtension> deviceExtensions;

    uint32_t apiVersion = VK_API_VERSION_1_3; 
};

class Context {
private:
    ContextCreateConfig m_Config{};
    uint32_t m_ApiVersion{};
    uint32_t m_GpuApiVersion{}; 

    VkInstance m_Instance{};
    VkPhysicalDevice m_PhysicalDevice{};
    VkDevice m_Device{};
    std::vector<QueueInfo> m_Queues;
    VkDebugUtilsMessengerEXT m_Callback{};
    std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
    std::vector<VkExtensionProperties> m_AvailableDeviceExtensions;

    VkPhysicalDeviceFeatures2 m_DeviceFeatures{};
    VkPhysicalDeviceVulkan11Features m_Features11{};
    VkPhysicalDeviceVulkan12Features m_Features12{};
    VkPhysicalDeviceVulkan13Features m_Features13{};
    VkPhysicalDeviceVulkan14Features m_Features14{};
public:
    Context() = default; 
    ~Context() = default;

    void Init(const ContextCreateConfig& config);
    void Uninit();

    VkDevice GetDevice() const { return m_Device; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
    VkInstance GetInstance() const { return m_Instance; }
    const QueueInfo& GetGraphicsQueue() { return m_Queues[0]; };
    uint32_t GetApiVersion() const { return m_ApiVersion; }
    uint32_t GetGpuApiVersion() const { return m_GpuApiVersion; }

    QueueInfo GetQueue(VkQueueFlagBits flags) const;
private:
    void _InitInstance();
    void _SelectPhysicalDevice();
    void _InitLogicalDevice();

    void _GetAvailableInstanceExtensions();
    void _GetAvailableDeviceExtensions();

    bool _InstanceExtensionIsAvailable(const char* name);
    bool _DeviceExtensionIsAvailable(const char* name);

#ifdef BEATENGINE_DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*
    );
#endif // BEATENGINE_DEBUG
};
}
