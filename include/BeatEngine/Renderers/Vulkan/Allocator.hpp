#pragma once

#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Context.h"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"
#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include <optional>

namespace VK {
class ImmediateExecutor;
class Allocator {
private:
    VmaAllocator m_Allocator{};
    VkPhysicalDevice m_PhysicalDevice{};
    VkDevice m_Device{};
    ImmediateExecutor* m_Executor;

    size_t m_BufferAllocs{}; 
    size_t m_ImageAllocs{};

    float m_MaxSamplerAnisotropy;
    ImageCache m_ImageCache;
public:
    Allocator() = default;
    ~Allocator() = default;

    void Init(const Context& ctx, ImmediateExecutor* executor);
    void Destroy(const Context& ctx);

    GPUBuffer CreateBuffer(size_t size, VkBufferUsageFlags bufUsage, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO);
    void DestroyBuffer(const GPUBuffer& buffer);

    AllocatedImage AllocateImage(VkImageCreateInfo info, std::optional<VmaAllocationCreateInfo> customAllocInfo);
    AllocatedImage CreateImage(VkImageCreateInfo info, const void* pixelData, bool cache = true);
    void DestroyImage(const AllocatedImage& image);
    void DestroyImage(ImageID id);
    void AttachImageData(ImageID id, const void* textureData, Vector2u offset = {}, Vector2u extent = {}, uint32_t layer = 0);
    void AttachImageData(AllocatedImage& image, const void* textureData, Vector2u offset = {}, Vector2u extent = {}, uint32_t layer = 0);
    void CopyImageToImage(ImageID srcID, ImageID dstID);

    VkDescriptorSetLayout GetBindlessDescSetLayout() { return m_ImageCache.BindlessSetMgr.GetSetLayout(); }
    void BindBindlessDescSet(VkCommandBuffer cmd, VkPipelineLayout layout);

    void SetMissingImageID(ImageID id) { m_ImageCache.SetMissingImageID(id); };

    size_t GetImageAllocations() const { return m_ImageAllocs; };
    size_t GetBufferAllocations() const { return m_BufferAllocs; }
    size_t GetAllAllocations() const { return m_ImageAllocs + m_BufferAllocs; }
};
}
