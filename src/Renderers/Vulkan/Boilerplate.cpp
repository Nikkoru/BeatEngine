#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/PipelineManager.h"
#include "BeatEngine/Util/Exception.h"

#include <SDL3/SDL_vulkan.h>
#include <cstdint>
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
        .objectType = type,
        .objectHandle = objectHandle,
        .pObjectName = name.c_str()
    };

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &objNameInfo));
}

VkInstance vkb::CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt, std::vector<const char*> pInstLayers) {
    VkInstance instance{ VK_NULL_HANDLE };

    uint32_t instanceExtensionCount{};

    auto instanceExtension = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
    
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
        pInstExt.emplace_back(instanceExtension[i]);

    VkApplicationInfo appInfo{ 
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName.c_str(),
        .pEngineName = "BeatEngine",
        .apiVersion = apiVersion,
    };

    VkInstanceCreateInfo instanceInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(pInstLayers.size()),
        .ppEnabledLayerNames = pInstLayers.data(), 
        .enabledExtensionCount = static_cast<uint32_t>(pInstExt.size()),
        .ppEnabledExtensionNames = pInstExt.data()
    };
    
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

    return instance;
}

VkPhysicalDevice vkb::CreatePhysicalDevice(VkInstance instance, uint32_t deviceIndex, VkPhysicalDeviceProperties* prop) {
    VkPhysicalDevice device{ VK_NULL_HANDLE };

    uint32_t devCount{};

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, nullptr));
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Available devices: {}", devCount);
    std::vector<VkPhysicalDevice> devices(devCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, devices.data()));
    
    VkPhysicalDeviceProperties2 devProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    if (deviceIndex >= devices.size()) {
        Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Not a valid index. autoselecting index 0");
        deviceIndex = 0;
    }

    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &devProperties);
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Selected device: {}", devProperties.properties.deviceName);
    if (prop)
        *prop = std::move(devProperties.properties);

    device = devices[deviceIndex];

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

std::vector<VkImageView> vkb::GetSwapchainImageViews(VkDevice device, std::vector<VkImage> images, VkFormat format) {
    std::vector<VkImageView> imageViews(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .flags = 0,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        };
        VK_CHECK(vkCreateImageView(device, &info, nullptr, &imageViews[i]));
        AddNameToVKObject(device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)imageViews[i], std::format("VkImageView_Swapchain{}", i));
    }
    return imageViews;
}

VkDevice vkb::CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily) {
    VkDevice device { VK_NULL_HANDLE };
    const float qfpriorities{ 1.0f };

    VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features features13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features12,
        .synchronization2 = true,        
        // .dynamicRendering = true
    };
	const VkPhysicalDeviceFeatures features10{ .samplerAnisotropy = VK_TRUE };
    
    // const VkPhysicalDeviceDynamicRenderingFeatures renderingFeatures{
    //     .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    //     .pNext = &features13,
    //     .dynamicRendering = VK_TRUE,
    // };
        
    const std::vector<const char*> deviceExtensions{ 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME 
    };

    VkDeviceCreateInfo deviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        // .pNext = &renderingFeatures,
        .pNext = &features13,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &features10
    };

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
    return device;
}

VkSwapchainKHR vkb::CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkSwapchainKHR oldSwapchain, VkFormat imageFormat) {
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

    VkSwapchainCreateInfoKHR swapchainInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
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
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
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

void vku::TransitionImage(PipelineManager mgr, VkCommandBuffer cmd, VkImage image, VkImageLayout curLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = curLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange = vkb::GetImageSubresourceRange(aspectMask);
    imageBarrier.image = image;

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
        .dstSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
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
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags
    };
}

VkImageViewCreateInfo vki::GetImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
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
        .renderArea = VkRect2D{ VkOffset2D { 0, 0 }, extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = colorAttachment,
        .pDepthAttachment = depthAttachment,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkSubmitInfo2 vki::GetSubmitInfo(VkCommandBufferSubmitInfo *cmdInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
    auto info = VkSubmitInfo2{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = cmdInfo
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
