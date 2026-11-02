#include "BeatEngine/Renderers/Vulkan/Allocator.hpp"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"
#include <volk.h>

namespace VK {
void Allocator::Init(const Context& ctx, ImmediateExecutor* executor) {
    m_Device = ctx.GetDevice();
    m_PhysicalDevice = ctx.GetPhysicalDevice();
    m_Executor = executor;

    VmaVulkanFunctions vkFunctions{}; // for stoping the -Wmissing-field-initializers we're going to set the variables from the obj and not a from initializer list
    vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vkFunctions.vkCreateImage = vkCreateImage;

    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = ctx.GetPhysicalDevice(),
        .device = ctx.GetDevice(),
        .preferredLargeHeapBlockSize{},
        .pAllocationCallbacks{},
        .pDeviceMemoryCallbacks{},
        .pHeapSizeLimit{},
        .pVulkanFunctions = &vkFunctions,
        .instance = ctx.GetInstance(),
        .vulkanApiVersion = VK_API_VERSION_1_4,
        .pTypeExternalMemoryHandleTypes{}
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));

    m_MaxSamplerAnisotropy = ctx.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;

    m_ImageCache.BindlessSetMgr.Init(ctx.GetDevice(), m_MaxSamplerAnisotropy);
}

void Allocator::Destroy(const Context& ctx) {

}

GPUBuffer Allocator::CreateBuffer(size_t size, VkBufferUsageFlags bufUsage, VmaMemoryUsage memUsage) {
    if (size <= 0) {
        AddVulkanLog("Invalid size for buffer, size is less than 0");
        THROW_RUNTIME_ERROR("Invalid size for buffer, size is less than 0");
    }

    const auto bufferInfo = VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .size = size,
        .usage = bufUsage,
        .sharingMode{},
        .queueFamilyIndexCount{},
        .pQueueFamilyIndices{}
    };

    const auto allocInfo = VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = memUsage,
        .requiredFlags{},
        .preferredFlags{},
        .memoryTypeBits{},
        .pool{},
        .pUserData{},
        .priority{}
    };

    GPUBuffer buf{};
    VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &buf.Buffer, &buf.Allocation, &buf.AllocationInfo));
    m_BufferAllocs++;

    if ((bufUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
        const auto deviceAddressInfo = VkBufferDeviceAddressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .pNext = nullptr,
            .buffer = buf.Buffer
        };

        buf.Address = vkGetBufferDeviceAddress(m_Device, &deviceAddressInfo);
    }

    buf.BufferSize = size;

    return buf;
}

void Allocator::DestroyBuffer(const GPUBuffer& buffer) {
    vmaDestroyBuffer(m_Allocator, buffer.Buffer, buffer.Allocation);
    m_BufferAllocs--;
    Logger::AddDebug(typeid(VulkanRenderer), "Current buffer allocs: {}", m_BufferAllocs);
}

AllocatedImage Allocator::AllocateImage(VkImageCreateInfo info, std::optional<VmaAllocationCreateInfo> customAllocInfo) {
    if (info.mipLevels <= 0) {
        info.mipLevels = 1;
    }

    if (info.arrayLayers <= 0) {
        info.arrayLayers = 1;
    }

    if (info.samples <= 0) {
        info.samples = VK_SAMPLE_COUNT_1_BIT;
    }

    if (info.imageType <= 0) {
        info.imageType = VK_IMAGE_TYPE_2D;
    }

    VkImageFormatProperties imageProperties;
    if (auto status = vkGetPhysicalDeviceImageFormatProperties(m_PhysicalDevice, info.format, info.imageType, info.tiling, info.usage, info.flags, &imageProperties); status != VK_SUCCESS) {
        Logger::AddError(typeid(VulkanRenderer), "Unable to create image; {}", string_VkResult(status));
        return {};
    }

    if (imageProperties.maxExtent.width < info.extent.width ||
        imageProperties.maxExtent.depth < info.extent.depth ||
        imageProperties.maxExtent.height < info.extent.height) {
        Logger::AddWarning(
            typeid(VulkanRenderer), 
            "Size mismatch; device can only create images from less or equal this size: (W: {}, H: {}, D: {}), but the required size was: (W: {}, H: {}, D: {})",
            imageProperties.maxExtent.width,
            imageProperties.maxExtent.height,
            imageProperties.maxExtent.depth,
            info.extent.width,
            info.extent.height,
            info.extent.depth
        );
    }

    if (info.mipLevels < 1) {
        Logger::AddWarning(typeid(VulkanRenderer), "Invalid mip levels");
    }

    static const auto defaultAllocInfo = VmaAllocationCreateInfo{
        .flags{},
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .preferredFlags{},
        .memoryTypeBits{},
        .pool{},
        .pUserData{},
        .priority{}
    };

    const auto allocInfo = customAllocInfo 
        ? customAllocInfo.value()
        : defaultAllocInfo;

    AllocatedImage image{};
    image.Format = info.format;
    image.Usage = info.usage;
    image.Extent = info.extent;
    image.MipLevels = info.mipLevels;

    
    VK_CHECK(vmaCreateImage(m_Allocator, &info, &allocInfo, &image.Image, &image.Allocation, nullptr));
    m_ImageAllocs++;

    bool shouldCreateView = ((info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);

    if (shouldCreateView) {
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (info.format == VK_FORMAT_D32_SFLOAT) { // TODO: support other depth formats
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        auto viewType = info.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        if (info.arrayLayers == 6) {
            viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }

        const auto viewCreateInfo = VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags{},
            .image = image.Image,
            .viewType = viewType,
            .format = info.format,
            .components{},
            .subresourceRange =
                VkImageSubresourceRange{
                    .aspectMask = aspectFlag,
                    .baseMipLevel = 0,
                    .levelCount = info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = info.arrayLayers,
                },
        };

        VK_CHECK(vkCreateImageView(m_Device, &viewCreateInfo, nullptr, &image.ImageView));
    }

    return image;
}

AllocatedImage Allocator::CreateImage(VkImageCreateInfo info, const void* pixelData, bool cache) {
    auto image = AllocateImage(info, std::nullopt);

    if (pixelData)
        AttachImageData(image, pixelData);

    image.Linear = m_ImageCache.BindlessSetMgr.GetLinearSampler();

    if (cache) {
        auto id = m_ImageCache.AddImage(m_Device, image);
        image.CachedID = id;
    }

    return image;
}

void Allocator::DestroyImage(const AllocatedImage& image) {
    vkDestroyImageView(m_Device, image.ImageView, nullptr);
    vmaDestroyImage(m_Allocator, image.Image, image.Allocation);
    m_ImageAllocs--;
    Logger::AddDebug(typeid(VulkanRenderer), "Current image allocs: {}", m_ImageAllocs);
}

void Allocator::DestroyImage(ImageID id) {
    auto& image = m_ImageCache.GetImage(id);
    DestroyImage(image);
}

void Allocator::AttachImageData(ImageID id, const void* textureData, Vector2u offset, Vector2u extent, uint32_t layer) {
    auto& image = m_ImageCache.GetImage(id);
    AttachImageData(image, textureData, offset, extent, layer);
}

void Allocator::AttachImageData(AllocatedImage& image, const void* pixelData, Vector2u offset, Vector2u extent, uint32_t layer) {
    int numChannels = 4;
    uint32_t dataSize{};

    VkExtent3D vkExtent{
        .width{},
        .height{},
        .depth = image.Extent.depth
    };

    if (extent == Vector2u{}) {
        dataSize = image.Extent.depth * image.Extent.height * image.Extent.width * numChannels;
        vkExtent.width = image.Extent.width;
        vkExtent.height = image.Extent.height;
    }
    else {
        dataSize = extent.X * extent.Y * numChannels;
        vkExtent.width = extent.X;
        vkExtent.height = extent.Y;
    }

    if ((extent + offset) >= Vector2u{ image.Extent.width, image.Extent.height })
        THROW_RUNTIME_ERROR("Dest position is outside of the image extent");

    const auto uploadBuffer = CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    memcpy(uploadBuffer.AllocationInfo.pMappedData, pixelData, dataSize);

    m_Executor->ImmediateSumbit([&](VkCommandBuffer cmd) {
        vku::TransitionImage(cmd, image.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        const auto copyRegion = VkBufferImageCopy{
            .bufferOffset{},
            .bufferRowLength{},
            .bufferImageHeight{},
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = layer,
                .layerCount = 1
            },
            .imageOffset = {
                .x = static_cast<int32_t>(offset.X),
                .y = static_cast<int32_t>(offset.Y),
                .z{}
            },
            .imageExtent = vkExtent
        };

        vkCmdCopyBufferToImage(cmd, uploadBuffer.Buffer, image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        if (image.MipLevels > 1)
            Logger::AddWarning(typeid(VulkanRenderer), "MipMaps are not implemented yet");

        vku::TransitionImage(cmd, image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });
    DestroyBuffer(uploadBuffer);
}

void Allocator::CopyImageToImage(ImageID, ImageID) {
}

void Allocator::BindBindlessDescSet(VkCommandBuffer cmd, VkPipelineLayout layout) {
    vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout,
            0,
            1,
            &m_ImageCache.BindlessSetMgr.GetSet(),
            0,
            nullptr
        );
    }
}
