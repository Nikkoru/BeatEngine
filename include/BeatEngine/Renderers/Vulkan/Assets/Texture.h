#pragma once 

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include <vulkan/vulkan_core.h>

namespace VK {
class Instance;
}

class VulkanTexture : public Texture {
private:
    friend class VK::Instance;
    AllocatedImage m_Image{};

    uint32_t m_ImageID{ NULL_IMAGE_ID };
public:
    VulkanTexture(AllocatedImage image) : m_Image(image) {}
    VulkanTexture() = default;

    bool IsInitialized() { return m_Image != AllocatedImage{}; }
    
    uint32_t GetID() { return m_Image.CachedID; }
protected:
    void MakeCopy(const Texture& other) override;
    void MakeMove(const Texture&& other) noexcept override;
};
