#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Instance.h"

ImageCache::ImageCache(VK::Instance& instance) : m_Instance(instance) {}

ImageID ImageCache::AddImage(AllocatedImage image) {
    return AddImage(GetFreeImageID(), image);    
}

ImageID ImageCache::AddImage(ImageID id, AllocatedImage image) {
    image.CachedID = id;

    if (id != m_Images.size()) {
        m_Images[id] = std::move(image);
    } else {
        m_Images.push_back(std::move(image));
    }

    BindlessSetMgr.AddImage(m_Instance.GetDevice(), id, image.ImageView);

    return id;
}

void ImageCache::DestroyImages() {

}
