#pragma once

#include "BeatEngine/Graphics/Vector2.h"
#include <filesystem>

#undef LoadImage

struct ImageData {
    ImageData() = default;
    ~ImageData();

    ImageData(ImageData&& o) = default;
    ImageData& operator=(ImageData&& o) = default;

    unsigned char* PixelData;
    Vector2i Size{};
    int Channels{};
    
    float* HDRPixelData;
    bool HDR{ false };
    int Comp{};

    bool ShouldFreeData{ false };

    static ImageData LoadImage(const std::filesystem::path& path);
};
