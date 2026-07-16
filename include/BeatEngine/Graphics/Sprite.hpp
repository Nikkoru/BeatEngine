#pragma once

#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"

class Sprite : public GraphicalElement {
public:
    Sprite() = default;

    void SetTexture(const AllocatedImage& texture);
    void SetTextureRect(const AllocatedImage& texture);

    void SetPixelPivot(const Vector2f& pos);
private:
    Vector2f m_Size{};
    Vector2f m_UV0{};
    Vector2f m_UV1{ 1.0f };

    Vector2f m_Pivot{};

    ImageID m_Texture{ NULL_IMAGE_ID };
};
