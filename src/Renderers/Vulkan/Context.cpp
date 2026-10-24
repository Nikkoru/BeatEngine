#include "BeatEngine/Renderers/Vulkan/Context.h"

#include <cstring>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Util/Exception.h"


namespace {
template<typename MainT, typename NewT>
inline void pNextChainPushFront(MainT* mainStruct, NewT* newStruct) {
    auto* newBase = reinterpret_cast<VkBaseOutStructure*>(newStruct);
    auto* mainBase = reinterpret_cast<VkBaseOutStructure*>(mainStruct);

    newBase->pNext = mainBase->pNext;
    mainBase->pNext = newBase;
}
}

namespace VK {
void Context::Init(const ContextCreateConfig& config) {
    m_Config = config;
    VK_CHECK(volkInitialize());
    _InitInstance();
    _SelectPhysicalDevice();
    _InitLogicalDevice();
}

QueueInfo Context::GetQueue(VkQueueFlagBits flags) const {
    return vkb::GetQueueFamily(m_PhysicalDevice, flags);
}

void Context::_InitInstance() {
    vkEnumerateInstanceVersion(&m_ApiVersion);
    _GetAvailableInstanceExtensions();

    std::vector<const char*> instanceExtensions = m_Config.instanceExtensions;

#ifdef BEATENGINE_DEBUG
    if(_InstanceExtensionIsAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
      instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // BEATENGINE_DEBUG
    if(_InstanceExtensionIsAvailable(VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME))
      instanceExtensions.push_back(VK_KHR_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

    std::vector<const char*> instanceLayers = m_Config.instanceLayers;
#ifdef BEATENGINE_DEBUG
    instanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif
    m_Instance = vkb::CreateInstance("BE_INT_TEST", m_ApiVersion, instanceExtensions, instanceLayers);

    volkLoadInstance(m_Instance);

#ifdef BEATENGINE_DEBUG
    if (vkCreateDebugUtilsMessengerEXT) {
      const VkDebugUtilsMessengerCreateInfoEXT dbgInfo{
          .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
          .pNext = nullptr,
          .flags{},
          .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
          .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
          .pfnUserCallback = Context::DebugCallback,  // <-- The callback function
          .pUserData = nullptr
      };
      VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_Instance, &dbgInfo, nullptr, &m_Callback));
    }
#endif
}

void Context::_SelectPhysicalDevice() {
    m_PhysicalDevice = vkb::CreatePhysicalDevice(m_Instance, m_AvailableDeviceExtensions);
}

void Context::_InitLogicalDevice() {
    m_Queues.clear();
    m_Queues.emplace_back(GetQueue(VK_QUEUE_GRAPHICS_BIT));

    pNextChainPushFront(&m_Features11, &m_Features12);
    pNextChainPushFront(&m_Features11, &m_Features13);

    _GetAvailableDeviceExtensions();

    std::vector<const char*> deviceExtensions;
    for (const auto& ext : m_Config.deviceExtensions) {
        if (_DeviceExtensionIsAvailable(ext.name)) {
            deviceExtensions.emplace_back(ext.name);
        }
        else if (ext.required) {
            AddVulkanLog("Required extension {} is not available!", ext.name);
            THROW_RUNTIME_ERROR("Required device extension not available, update driver!");
        }
        else {
            AddVulkanLog("Optional extension {} is not available, skipping", ext.name);
        }

        if (ext.featureStruct) {
            pNextChainPushFront(&m_Features11, ext.featureStruct);
        }
    }

    m_DeviceFeatures.pNext = &m_Features11;

    m_Device = vkb::CreateDevice(m_PhysicalDevice, m_Queues[0].familyIndex, &m_DeviceFeatures, deviceExtensions);
}

void Context::_GetAvailableInstanceExtensions() {
    uint32_t count{};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    m_AvailableInstanceExtensions.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, m_AvailableInstanceExtensions.data());
}

void Context::_GetAvailableDeviceExtensions() {
    uint32_t count{};
    VK_CHECK(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &count, nullptr));
    m_AvailableDeviceExtensions.resize(count);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &count, m_AvailableDeviceExtensions.data()));
}

bool Context::_InstanceExtensionIsAvailable(const char* name) {
    for (const auto& ext : m_AvailableInstanceExtensions) {
        if (strcmp(name, ext.extensionName) == 0)
            return true;
    }

    return false;
}

bool Context::_DeviceExtensionIsAvailable(const char* name) {
    for (const auto& ext : m_AvailableDeviceExtensions) {
        if (strcmp(name, ext.extensionName) == 0)
            return true;
    }

    return false;
}

#ifdef BEATENGINE_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL Context::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*
) {
    return VK_FALSE;
}
#endif // BEATENGINE_DEBUG
}; // namespace VK
