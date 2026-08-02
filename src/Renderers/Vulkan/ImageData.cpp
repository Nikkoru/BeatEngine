#include "BeatEngine/Renderers/Vulkan/ImageData.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <BeatEngine/System/String.hpp>

ImageData::~ImageData() {
    if (ShouldFreeData) {
        stbi_image_free(PixelData);
        stbi_image_free(HDRPixelData);
    }
}

ImageData ImageData::LoadImage(const std::filesystem::path& path) {
    ImageData data{};
	String pathStr = path.wstring();

    data.ShouldFreeData = true;
    if (stbi_is_hdr(pathStr.ToCString(true))) {
        data.HDR = true;
        data.HDRPixelData = stbi_loadf(pathStr.ToCString(true), &data.Size.X, &data.Size.Y, &data.Comp, 4);
    }
    else {
        data.PixelData = stbi_load(pathStr.ToCString(true), &data.Size.X, &data.Size.Y, &data.Channels, 4);
    }
    data.Channels = 4;

    return data;
}
