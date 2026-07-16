#pragma once

#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/BindlessSetManager.h"
#include <vector>

namespace VK {
class Instance;
}
class ImageCache {
private:
    std::vector<AllocatedImage> m_Images;
    VK::Instance& m_Instance;

    ImageID m_MissingImageID{ NULL_IMAGE_ID };
public:
    BindlessSetManager BindlessSetMgr{};
public:
    ImageCache(VK::Instance& instance);

    ImageID AddImage(AllocatedImage image);
    ImageID AddImage(ImageID id, AllocatedImage image);
    const AllocatedImage& GetImage(ImageID id) { return m_Images[id]; };

    void SetMissingImageID(ImageID id) { m_MissingImageID = id; }

    ImageID GetFreeImageID() { return m_Images.size(); }

    void DestroyImages(); 
};
