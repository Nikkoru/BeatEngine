#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Context.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"


ImageID ImageCache::AddImage(VkDevice device, AllocatedImage image) {
    return AddImage(device, GetFreeImageID(), image);    
}

ImageID ImageCache::AddImage(VkDevice device, ImageID id, AllocatedImage image) {
    Logger::AddDebug(typeid(VulkanRenderer), "Adding image to id {}", id);
    image.CachedID = id;

    if (id != m_Images.size()) {
        m_Images[id] = std::move(image);
    } else {
        m_Images.push_back(std::move(image));
    }

    BindlessSetMgr.AddImage(device, id, image.ImageView);

    return id;
}

AllocatedImage& ImageCache::GetImage(ImageID id) {
    if (id == NULL_IMAGE_ID) {
        Logger::AddWarning(typeid(VulkanRenderer), "ID is NULL_IMAGE_ID");
        return m_Images[0];
    }

    if (m_Images.size() <= id) {
        Logger::AddError(typeid(VulkanRenderer), "requested ID ({}) is not available!, cached images: {}", id, m_Images.size());
        return m_Images[m_MissingImageID];
    }

    return m_Images[id];
};

void ImageCache::DestroyImages(VK::Allocator& allocator) {
    for (const auto& image : m_Images) {
        allocator.DestroyImage(image);
    }
}
