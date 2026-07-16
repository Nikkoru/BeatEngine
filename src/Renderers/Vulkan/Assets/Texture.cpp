#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Asset/Texture.h"

void VulkanTexture::MakeCopy(const Texture& other) {
    auto otherCast = static_cast<const VulkanTexture*>(&other);

    if (otherCast) {
        this->m_Image = otherCast->m_Image;
    }
}

void VulkanTexture::MakeMove(const Texture&& other) noexcept {
    auto otherCast = static_cast<const VulkanTexture*>(std::move(&other));
    if (otherCast) {
        this->m_Image = std::move(otherCast->m_Image);
    }

}
