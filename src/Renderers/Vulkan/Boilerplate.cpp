#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"

#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <volk.h>
#include <format>

void VK_CHECK_SOURCE(VkResult result, const std::source_location location) {
    if (result) {
        auto msg = std::format("Vulkan error: {} From: {}:{}:{}", string_VkResult(result), location.function_name(), location.line(), location.column());
        Logger::AddCritical("", msg);
        THROW_RUNTIME_ERROR(msg);
    }
}

void AddNameToVKObject(VkDevice device, VkObjectType type, uint64_t objectHandle, std::string name) {
    VkDebugUtilsObjectNameInfoEXT objNameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = type,
        .objectHandle = objectHandle,
        .pObjectName = name.c_str()
    };

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &objNameInfo));
}
bool VK_CHECK_SWAPCHAIN_SOURCE(VkResult result,
                               const std::source_location location) {
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "",
                   "Swapchain out of date. Requesting update");
    return true;
  } else {
    VK_CHECK_SOURCE(result, location);
    return false;
  }
}

VkInstance vkb::CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt, std::vector<const char*> pInstLayers) {
    VkInstance instance{ VK_NULL_HANDLE };

    uint32_t instanceExtensionCount{};

    auto instanceExtension = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
    
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
        pInstExt.emplace_back(instanceExtension[i]);

    VkApplicationInfo appInfo{ 
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = appName.c_str(),
        .applicationVersion{},
        .pEngineName = "BeatEngine",
        .engineVersion{},
        .apiVersion = apiVersion,
    };

    VkInstanceCreateInfo instanceInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(pInstLayers.size()),
        .ppEnabledLayerNames = pInstLayers.data(), 
        .enabledExtensionCount = static_cast<uint32_t>(pInstExt.size()),
        .ppEnabledExtensionNames = pInstExt.data()
    };
    
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

    return instance;
}

VkPhysicalDevice vkb::CreatePhysicalDevice(VkInstance instance, std::vector<VkExtensionProperties>& availableExts, uint32_t deviceIndex, VkPhysicalDeviceProperties* prop) {
    VkPhysicalDevice device{ VK_NULL_HANDLE };

    uint32_t devCount{};

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, nullptr));
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Available devices: {}", devCount);
    std::vector<VkPhysicalDevice> devices(devCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, devices.data()));

    
    VkPhysicalDeviceProperties2 devProperties{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = nullptr,
        .properties{}
    };
    if (deviceIndex >= devices.size()) {
        Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Not a valid index. autoselecting index 0");
        deviceIndex = 0;
    }

    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &devProperties);
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Selected device: {}", devProperties.properties.deviceName);
    if (prop)
        *prop = std::move(devProperties.properties);

    device = devices[deviceIndex];

    uint32_t extensionCount{};
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
    availableExts.resize(extensionCount);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExts.data()));

    return device;
}

uint32_t vkb::GetQueueFamily(VkPhysicalDevice device) {
    uint32_t queueFamily{};
    uint32_t count{};

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    for (size_t i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamily = i;
			break;
		}
	}

    return queueFamily;
}

std::vector<VkImage> vkb::GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain) {
    uint32_t imageCount{ 0 };
    std::vector<VkImage> images; 
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
    images.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data()));
    
    auto i = 0;
    for (const auto& image : images) {
        AddNameToVKObject(device, VK_OBJECT_TYPE_IMAGE, uint64_t(image), std::format("VkImage_Swapchain{}", i));
        i++;
    }

    return images;
}

std::vector<VkImageView> vkb::GetSwapchainImageViews(VkDevice device, std::vector<VkImage>& images, VkFormat format) {
    std::vector<VkImageView> imageViews(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components{},
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel{},
                .levelCount = 1,
                .baseArrayLayer{},
                .layerCount = 1,
            }
        };
        VK_CHECK(vkCreateImageView(device, &info, nullptr, &imageViews[i]));
        AddNameToVKObject(device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)imageViews[i], std::format("VkImageView_Swapchain{}", i));
    }
    return imageViews;
}

VkDevice vkb::CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily, std::vector<const char*>& requestedExts, std::vector<VkExtensionProperties>& deviceExts) {
    VkDevice device { VK_NULL_HANDLE };
    const float qfpriorities{ 1.0f };

    VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    VkPhysicalDeviceVulkan12Features features12{}; // for stoping the -Wmissing-field-initializers we're going to set the variables from the obj and not from a initializer list
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = nullptr;
    features12.descriptorIndexing = true;
    features12.shaderSampledImageArrayNonUniformIndexing = true;
    features12.descriptorBindingSampledImageUpdateAfterBind = true;
    features12.descriptorBindingStorageImageUpdateAfterBind = true;
    features12.descriptorBindingPartiallyBound = true;
    features12.descriptorBindingVariableDescriptorCount = true;
    features12.runtimeDescriptorArray = true;
    features12.bufferDeviceAddress = true;
    
    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.pNext = &features12;
    features13.synchronization2 = true;        
    features13.dynamicRendering = true;

	VkPhysicalDeviceFeatures features10{}; 
    features10.imageCubeArray = VK_TRUE;
    features10.geometryShader = VK_TRUE;
    features10.depthClamp = VK_TRUE;
    features10.samplerAnisotropy = VK_TRUE;
    features10.largePoints = VK_TRUE;

    auto shaderRelaxedFeatures = VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR,
        .pNext = &features13,
        .shaderRelaxedExtendedInstruction = VK_TRUE
    };

    const VkPhysicalDeviceShaderObjectFeaturesEXT shaderFeatures {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        .pNext = &shaderRelaxedFeatures,
        .shaderObject = VK_TRUE
    };
    requestedExts.emplace_back(
        VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    requestedExts.emplace_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME);

    requestedExts.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    {
        bool supported = false;
        for (const auto& requestedExt : requestedExts) {
            supported = false;
            for (const auto& deviceExt : deviceExts) {
                if (strcmp(requestedExt, deviceExt.extensionName) == 0) {
                    supported = true;
                    break;
                }
            }
            if (!supported) {
                Logger::AddError(typeid(VulkanRenderer), "Requested device extension \"{}\" is not available on this device", requestedExt);
            }
        }

        if (!supported) {
            THROW_RUNTIME_ERROR("One of the requested device extensions is not available");
        }
    }


    VkDeviceCreateInfo deviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &shaderFeatures,
        .flags{},
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledLayerCount{},
        .ppEnabledLayerNames{},
        .enabledExtensionCount = static_cast<uint32_t>(requestedExts.size()),
        .ppEnabledExtensionNames = requestedExts.data(),
        .pEnabledFeatures = &features10,
    };

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
    return device;
}

VkSwapchainKHR vkb::CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain, VkFormat imageFormat) {
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

    VkSwapchainCreateInfoKHR swapchainInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags{},
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{
            .width = width,
            .height = height
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount{},
        .pQueueFamilyIndices{},
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped{},
        .oldSwapchain{}
    };
    if (oldSwapchain)
        swapchainInfo.oldSwapchain = oldSwapchain;

    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

    return swapchain;
}

VkImageSubresourceRange vkb::GetImageSubresourceRange(VkImageAspectFlags flags) {
    return {
        .aspectMask = flags,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
}

VkPipelineLayout vkb::CreatePipelineLayout(VkDevice device, VkDescriptorSetLayout descLayout, VkPushConstantRange range) {

    const auto info = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .setLayoutCount = 1,
        .pSetLayouts = &descLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &range
    };

    VkPipelineLayout layout;
    VK_CHECK(vkCreatePipelineLayout(device, &info, nullptr, &layout));

    return layout;
}

void vku::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout curLayout, VkImageLayout newLayout) {
    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 imageBarrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = curLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex{},
        .dstQueueFamilyIndex{},
        .image = image,
        .subresourceRange = vkb::GetImageSubresourceRange(aspectMask),
    };

    VkDependencyInfo depInfo {};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

void vku::CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) {
    VkImageBlit2 blitRegion{ 
        .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, 
        .pNext = nullptr,
        .srcSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcOffsets{},
        .dstSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstOffsets{},
    };

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	VkBlitImageInfo2 blitInfo{ 
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, 
        .pNext = nullptr,
        .srcImage = source,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = destination,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1,
        .pRegions = &blitRegion,
        .filter = VK_FILTER_LINEAR
    };

	vkCmdBlitImage2(cmd, &blitInfo);
}

VkImageCreateInfo vki::GetImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags,
        .sharingMode{},
        .queueFamilyIndexCount{},
        .pQueueFamilyIndices{},
        .initialLayout{}
    };
}

VkImageViewCreateInfo vki::GetImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components{},
        .subresourceRange {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
}

VkRenderingAttachmentInfo vki::GetRenderingAttachmentInfo(VkImageView target, VkClearValue* clear, VkImageLayout layout) {
    VkRenderingAttachmentInfo colorAttachment {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = target,
        .imageLayout = layout,
        .resolveMode{},
        .resolveImageView{},
        .resolveImageLayout{},
        .loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear ? *clear : VkClearValue(),
    };

    return colorAttachment;
}
VkRenderingInfo vki::GetRenderingInfo(VkExtent2D extent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
    VkRenderingInfo renderInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags{},
        .renderArea = VkRect2D{ VkOffset2D { 0, 0 }, extent },
        .layerCount = 1,
        .viewMask{},
        .colorAttachmentCount = 1,
        .pColorAttachments = colorAttachment,
        .pDepthAttachment = depthAttachment,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkSubmitInfo2 vki::GetSubmitInfo(const VkCommandBufferSubmitInfo *cmdInfo, const VkSemaphoreSubmitInfo* signalSemaphoreInfo, const VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
    auto info = VkSubmitInfo2{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .flags{},
        .waitSemaphoreInfoCount{},
        .pWaitSemaphoreInfos{},
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmdInfo,
        .signalSemaphoreInfoCount{},
        .pSignalSemaphoreInfos{}
    };

    info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = waitSemaphoreInfo;

    info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signalSemaphoreInfo;

    return info;
}

VkCommandBufferSubmitInfo vki::GetCommandBufferSubmitInfo(VkCommandBuffer cmd) {
	return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0
    };
}

VkCommandPoolCreateInfo vki::GetCommandPoolCreateInfo(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex
    };
}

VkCommandBufferAllocateInfo vki::GetCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = commandBufferCount
    };
}

VkSemaphoreSubmitInfo vki::GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageFlags, VkSemaphore semaphore) {
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageFlags,
        .deviceIndex = 0
    };
}

VkPipelineInputAssemblyStateCreateInfo vki::GetPipelineInputAssemblyStateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable) {
    VkPipelineInputAssemblyStateCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .topology = topology,
        .primitiveRestartEnable = primitiveRestartEnable
    };

    return info;
}
