#pragma once

#include "BeatEngine/Graphics/Rect.hpp"

struct Glyph {
    float Advance{};
    int lbsDelta{};
    int rsbDelta{};
    FloatRect Bounds{};
    IntRect TextureRect{};
};
