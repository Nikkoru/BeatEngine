#pragma once

#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/BindlessSetManager.h"
#include <cassert>
#include <vector>

namespace VK {
class Context;
class Allocator;
}
class ImageCache {
private:
    std::vector<AllocatedImage> m_Images;

    ImageID m_MissingImageID{ NULL_IMAGE_ID };
public:
    BindlessSetManager BindlessSetMgr{};
public:
    ImageCache() = default;
    ~ImageCache() = default; 

    ImageID AddImage(VkDevice device, AllocatedImage image);
    ImageID AddImage(VkDevice device, ImageID id, AllocatedImage image);
    AllocatedImage& GetImage(ImageID id);

    void SetMissingImageID(ImageID id) { m_MissingImageID = id; }

    ImageID GetFreeImageID() { return m_Images.size(); }

    void DestroyImages(VK::Allocator& allocator); 
};
