#pragma once

#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"

class GameContext {
public:
    GameFlags GFlags = GameFlags_None;
    ViewFlags VFlags = ViewFlags_None;
public:
    GameContext() = default;
};
