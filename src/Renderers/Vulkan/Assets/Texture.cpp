#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include <vulkan/vulkan_core.h>

VulkanTexture& VulkanTexture::operator=(VulkanTexture other) {
    this->m_CacheID = other.m_CacheID;
    this->m_ImGuiDrawData = other.m_ImGuiDrawData;
    this->m_Size = other.m_Size;

    return *this;
}

ImTextureID VulkanTexture::GetImGuiTexture(GraphicsManager& mgr) {
    if (m_ImGuiDrawData == VkDescriptorSet{}) {
        auto renderer = std::dynamic_pointer_cast<VulkanRenderer>(mgr.GetRenderer());
        auto image = renderer->GetImageFromID(m_CacheID);

        m_ImGuiDrawData = ImGui_ImplVulkan_AddTexture(image.Linear, image.ImageView, VK_IMAGE_LAYOUT_GENERAL);
    }

    return (ImTextureID)m_ImGuiDrawData;
}

void VulkanTexture::MakeCopy(const Texture& other) {
    auto otherCast = static_cast<const VulkanTexture*>(&other);

    if (otherCast) {
        this->m_CacheID = otherCast->m_CacheID;
        this->m_ImGuiDrawData = otherCast->m_ImGuiDrawData;
        this->m_Size = otherCast->m_Size;
    }
}

void VulkanTexture::MakeMove(const Texture&& other) noexcept {
    auto otherCast = dynamic_cast<const VulkanTexture*>(std::move(&other));
    if (otherCast) {
        this->m_CacheID = std::move(otherCast->m_CacheID);
        this->m_ImGuiDrawData = std::move(otherCast->m_ImGuiDrawData);
        this->m_Size = std::move(otherCast->m_Size);
    }

}
