#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Instance.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"

ImageCache::ImageCache(VK::Instance& instance) : m_Instance(instance) {}

ImageID ImageCache::AddImage(AllocatedImage image) {
    return AddImage(GetFreeImageID(), image);    
}

ImageID ImageCache::AddImage(ImageID id, AllocatedImage image) {
    Logger::AddDebug(typeid(VulkanRenderer), "Adding image to id {}", id);
    image.CachedID = id;

    if (id != m_Images.size()) {
        m_Images[id] = std::move(image);
    } else {
        m_Images.push_back(std::move(image));
    }

    BindlessSetMgr.AddImage(m_Instance.GetDevice(), id, image.ImageView);

    return id;
}

const AllocatedImage& ImageCache::GetImage(ImageID id) {
    if (id == NULL_IMAGE_ID) {
        Logger::AddCritical(typeid(VulkanRenderer), "ID is NULL_IMAGE_ID");
        THROW_RUNTIME_ERROR("requested ID for ImageCache is NULL_IMAGE_ID");
    }

    if (m_Images.size() <= id) {
        Logger::AddCritical(typeid(VulkanRenderer), "requested ID ({}) is not available!, cached images: {}", id, m_Images.size());
        THROW_RUNTIME_ERROR("Requested ID for ImageCache is not valid");
    }

    return m_Images[id];
};

void ImageCache::DestroyImages() {
    for (const auto& image : m_Images) {
        m_Instance.DestroyImage(image);
    }
}
