#pragma once

#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Context.h"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"

namespace VK {
class Allocator {
private:
    VmaAllocator m_Allocator{};
    size_t m_BufferAllocs{}; 
    size_t m_ImageAllocs{};
public:
    Allocator() = default;
    ~Allocator() = default;

    void Init(const Context& ctx);
    void Destroy(const Context& ctx);

    GPUBuffer CreateBuffer(size_t size, VkBufferUsageFlags bufUsage, VmaMemoryUsage memUsage);
    void DestroyBuffer(const GPUBuffer& buffer);

    AllocatedImage CreateImage(VkImageCreateInfo info, std::optional<VmaAllocationCreateInfo> customAllocInfo);
};
}
