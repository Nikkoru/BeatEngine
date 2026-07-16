#include "BeatEngine/Renderers/Vulkan/ImageData.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

ImageData::~ImageData() {
    if (ShouldFreeData) {
        stbi_image_free(PixelData);
        stbi_image_free(HDRPixelData);
    }
}

ImageData ImageData::LoadImage(const std::filesystem::path& path) {
    ImageData data{};
    data.ShouldFreeData = true;
    if (stbi_is_hdr(path.c_str())) {
        data.HDR = true;
        data.HDRPixelData = stbi_loadf(path.c_str(), &data.Size.X, &data.Size.Y, &data.Comp, 4);
    }
    else {
        data.PixelData = stbi_load(path.c_str(), &data.Size.X, &data.Size.Y, &data.Channels, 4);
    }
    data.Channels = 4;

    return data;
}
