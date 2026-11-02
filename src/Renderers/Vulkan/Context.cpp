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

void Context::Uninit() {
    AddVulkanLog("Uninitializing Vulkan handlers");

    vkDestroyDebugUtilsMessengerEXT(m_Instance, m_Callback, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void Context::InitImGui() {

}

QueueInfo Context::GetQueue(VkQueueFlagBits flags) const {
    return vkb::GetQueue(m_PhysicalDevice, flags);
}

void Context::_InitInstance() {
    if (m_Config.apiVersion != 0)
        m_ApiVersion = m_Config.apiVersion;
    else
        vkEnumerateInstanceVersion(&m_ApiVersion);
    AddVulkanLog("Using API version: {}.{}.{}", VK_VERSION_MAJOR(m_ApiVersion), VK_VERSION_MINOR(m_ApiVersion), VK_VERSION_PATCH(m_ApiVersion));
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
    m_PhysicalDevice = vkb::CreatePhysicalDevice(m_Instance, m_AvailableDeviceExtensions, -1, &m_DeviceProperties);
}

void Context::_InitLogicalDevice() {
    m_Queues.clear();
    m_Queues.emplace_back(GetQueue(VK_QUEUE_GRAPHICS_BIT));

    _PopulateFeatures();

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
            continue;
        }

        if (ext.featureStruct != nullptr) {
            pNextChainPushFront(&m_Features11, ext.featureStruct);
        }
    }
    m_DeviceFeatures = VkPhysicalDeviceFeatures2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features{}
    };

    m_DeviceFeatures.pNext = &m_Features11;
    vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &m_DeviceFeatures);

    m_Device = vkb::CreateDevice(m_PhysicalDevice, m_Queues[0].familyIndex, &m_DeviceFeatures, deviceExtensions);
    vkGetDeviceQueue(m_Device, m_Queues[0].familyIndex, m_Queues[0].queueIndex, &m_Queues[0].queue);
}

void Context::_PopulateFeatures() {
    m_Features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    m_Features11.pNext = nullptr;

    m_Features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    m_Features12.pNext = nullptr;
    m_Features12.descriptorIndexing = true;
    m_Features12.shaderSampledImageArrayNonUniformIndexing = true;
    m_Features12.descriptorBindingSampledImageUpdateAfterBind = true;
    m_Features12.descriptorBindingStorageImageUpdateAfterBind = true;
    m_Features12.descriptorBindingPartiallyBound = true;
    m_Features12.descriptorBindingVariableDescriptorCount = true;
    m_Features12.runtimeDescriptorArray = true;
    m_Features12.bufferDeviceAddress = true;
    m_Features12.timelineSemaphore = true;

    m_Features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    m_Features13.pNext = nullptr;
    m_Features13.synchronization2 = true;        
    m_Features13.dynamicRendering = true;
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

namespace {
std::string GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return "Verbose";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        return "Info";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return "Warning";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return "Error";
    default:
        return "Invalid";
    }
}

std::string GetDebugSeverityColorStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return "\e[30;47m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        return "\e[30;106m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return "\e[0;103m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return "\e[0;101m";
    default:
        return "\033[0m";
    }
}

std::string GetDebugTypeStr(VkDebugUtilsMessageTypeFlagsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        return "General";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        return "Validation";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
        return "Device Address Binding";
    default:
        return "Invalid";
    }
}
}

#ifdef BEATENGINE_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL Context::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*
) {
    Logger::AddLog("\x1b[0;41mVulkan Debug\033[0m", "", "{}", callbackData->pMessage);
    Logger::AddLog("\x1b[0;41mVulkan Debug\033[0m", "", "    Type = {}", GetDebugTypeStr(type));
    Logger::AddLog("\x1b[0;41mVulkan Debug\033[0m", "", "    Severity = {}{}\033[0m", GetDebugSeverityColorStr(severity), GetDebugSeverityStr(severity));
    Logger::AddLog("\x1b[0;41mVulkan Debug\033[0m", "", "    Objects: {}", callbackData->objectCount);
    for (uint32_t i = 0; i < callbackData->objectCount; i++) {
        auto name = "No name available";
        if (callbackData->pObjects[i].pObjectName)
            name = callbackData->pObjects[i].pObjectName;

        Logger::AddLog("\x1b[0;41mVulkan Debug\033[0m", "", "        {:#x} ({})", callbackData->pObjects[i].objectHandle, name);
    }
    return VK_FALSE;
}
#endif // BEATENGINE_DEBUG
}; // namespace VK
