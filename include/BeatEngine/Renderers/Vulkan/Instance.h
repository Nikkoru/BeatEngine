#pragma once

#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"
#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include "BeatEngine/Renderers/Vulkan/ImmediateExecutor.h"
#include "BeatEngine/Renderers/Vulkan/Swapchain.h"
#include "BeatEngine/Renderers/Vulkan/UninitQueue.h"
#include <memory>
#include <optional>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

class GameContext;
class VulkanRenderer;
namespace VK {
class Instance {
private:
    friend class VulkanRenderer;
    GameContext* m_Context{ nullptr };
    Core m_Core{};
    Swapchain m_Swapchain{};

    ImageCache m_ImageCache;

    ImmediateExecutor m_Executor{};

    UninitQueue m_Uninitializers{};

    VulkanTexture m_ErrorTexture{};
    VulkanTexture m_WhiteTexture{};

    float m_MaxSamplerAnisotropy{};

    VkSampleCountFlagBits m_SupportedSampleCounts{};
    VkSampleCountFlagBits m_HighestSupportedSample{};

    uint16_t m_ImageAllocs{};
    uint16_t m_BufferAllocs{};

    uint32_t m_FrameNumber{};
public:
    Instance() : m_ImageCache(*this) {}

    void Init(GameContext* context, std::string appName, uint32_t deviceIndex, std::shared_ptr<BaseWindow> window, VSyncMode vSync = Disable);
    void Uninit();

    void AttachImageData(ImageID textureID, const void* pixelData, Vector2u offset = { 0, 0 }, Vector2u extent = { 0, 0 }, uint32_t layer = 0);
    void AttachImageData(AllocatedImage& texture, const void* pixelData, Vector2u offset = { 0, 0 }, Vector2u extent = { 0, 0 }, uint32_t layer = 0);

    const AllocatedImage& GetImageFromID(ImageID id) { return m_ImageCache.GetImage(id); }

    uint16_t GetImageAllocations() { return m_ImageAllocs; }
    uint16_t GetBufferAllocations() { return m_BufferAllocs; }

    VkCommandBuffer BeginFrame();
    void EndFrame(VkCommandBuffer cmd, AllocatedImage& drawImage);
    void ClearImage(VkCommandBuffer cmd, VkClearColorValue clearColor, VkImageSubresourceRange imageRange);

    AllocatedImage CreateImage(VkImageCreateInfo info, const void* pixelData, bool cache = true);
    void CopyImageToImage(ImageID srcID, ImageID dstID);
    void DestroyImage(const ImageID imageID);
    void DestroyImage(const AllocatedImage& image);
    AllocatedImage CreateDrawImage(Vector2u size);

    VkPhysicalDeviceProperties& GetDeviceProperties() { return m_Core.DeviceProperties; }
    VkDevice GetDevice() { return m_Core.Device; }

    uint32_t GetCurrentFrameIndex() const { return m_FrameNumber % FRAME_OVERLAP; }
    uint32_t GetCurrentFrameNumber() const { return m_FrameNumber; }

    VulkanTexture& GetErrorTexture() { return m_ErrorTexture; }
    VulkanTexture& GetWhiteTexture() { return m_WhiteTexture; }

    void CheckDeviceCapabilities();

    void WaitIdle();

    bool NeedsRecreateSwapchain() { return m_Swapchain.IsOutdated(); }
    Vector2u GetSwapchainExtent() { return m_Swapchain.GetExtent(); }
    VkFormat GetSwapchainFormat() { return m_Swapchain.GetFormat(); }
    size_t GetSwapchainImagesSize() { return m_Swapchain.GetImagesSize(); }
    VkImage& GetSwapchainImage() { return m_Swapchain.GetImage(GetCurrentFrameIndex()); }
    VkImageView& GetSwapchainImageView() { return m_Swapchain.GetImageView(GetCurrentFrameIndex()); }
    void RecreateSwapchain(std::shared_ptr<BaseWindow> window, Vector2u size);

    ImageID AddImageToCache(AllocatedImage& image);

    GPUBuffer CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);
    void DestroyBuffer(const GPUBuffer& buffer);

    VkDescriptorSetLayout GetBindlessDescSetLayout() { return m_ImageCache.BindlessSetMgr.GetSetLayout(); }
    void BindBindlessDescSet(VkCommandBuffer cmd, VkPipelineLayout layout);
private:
    friend class VulkanRenderer;

    void InitVulkan(std::shared_ptr<BaseWindow> window, const char* appName, uint32_t deviceIndex);
    void InitImGui(std::shared_ptr<BaseWindow> window);

    AllocatedImage CreateImageRaw(
        VkImageCreateInfo info, 
        std::optional<VmaAllocationCreateInfo> customAllocInfo = std::nullopt
    );


    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
};
}
