#pragma once

#include "BeatEngine/Graphics/Vector2.h"
#include <string>
namespace Helper {
    bool ParseBoolFromString(const char* str);

    std::string SecondsToStringH(float seconds);

    bool RectHovered(Vector2f posMin, Vector2f posMax, Vector2i mousePos = { -1, -1 }, bool showCondition = false);
}
