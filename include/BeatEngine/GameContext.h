#pragma once

#include <string>
#include <typeindex>

#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Graphics/Vector2.h"

class GameContext {
public:
    EnvFlags EFlags = EnvFlags_None;
    GameFlags GFlags = GameFlags_None;
    ViewFlags VFlags = ViewFlags_None;
    Vector2u WindowSize{};
    std::type_index ActiveView = typeid(nullptr);
    const std::string ProgramName{};
public:
    GameContext(): GameContext("BeatEngine Game") {}
    GameContext(std::string name) : ProgramName(name) {} 
};
