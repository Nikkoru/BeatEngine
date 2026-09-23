#pragma once

#include <string>
#include <typeindex>

#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/AppFlags.hpp"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Graphics/Vector2.h"

class AppContext {
public:
    EnvFlags EFlags = EnvFlags_None;
    AppFlags GFlags = AppFlags_None;
    ViewFlags VFlags = ViewFlags_None;
    Vector2u WindowSize{};
    std::type_index ActiveView = typeid(nullptr);
    const std::string ProgramName{};
public:
    AppContext(): AppContext("BeatEngine App") {}
    AppContext(std::string name) : ProgramName(name) {} 
};
