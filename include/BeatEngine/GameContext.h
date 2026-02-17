#pragma once

#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Graphics/Vector2.h"

#include <typeindex>

class GameContext {
public:
    GameFlags GFlags = GameFlags_None;
    ViewFlags VFlags = ViewFlags_None;
    Vector2u WindowSize{};
    std::type_index ActiveView = typeid(nullptr);
public:
    GameContext() = default;
};
