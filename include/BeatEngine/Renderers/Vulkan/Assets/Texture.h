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
    friend class VulkanRenderer;
    VkDescriptorSet m_ImGuiDrawData{};
public:
    VulkanTexture(AllocatedImage image) : Texture(image.CachedID) {}
    VulkanTexture(VulkanTexture& other) : Texture(other.m_CacheID, other.m_Size), m_ImGuiDrawData(other.m_ImGuiDrawData) {}
    VulkanTexture(VulkanTexture&& other) : Texture(std::move(other.m_CacheID), std::move(other.m_Size)), m_ImGuiDrawData(std::move(other.m_ImGuiDrawData)) {}
    VulkanTexture(Texture& texture) : Texture(texture) {}
    VulkanTexture(Texture&& texture) : Texture(std::move(texture)) {}
    VulkanTexture() = default;

    VulkanTexture& operator=(VulkanTexture other);

    bool IsInitialized() { return m_CacheID != NULL_IMAGE_ID; }
    bool IsValid() override { return IsInitialized(); }
    ImTextureID GetImGuiTexture(GraphicsManager& mgr) override;
protected:
    void MakeCopy(const Texture& other) override;
    void MakeMove(const Texture&& other) noexcept override;
};
