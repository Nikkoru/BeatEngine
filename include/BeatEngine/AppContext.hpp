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
    AppFlags AFlags = AppFlags_None;
    ViewFlags VFlags = ViewFlags_None;
    Vector2u WindowSize{};
    std::type_index ActiveView = typeid(nullptr);
    const std::string ProgramName{};
public:
    AppContext(): AppContext("BeatEngine App") {}
    AppContext(std::string name) : ProgramName(name) {} 

    void AddEFlags(EnvFlags flags) { EFlags |= flags; }
    void AddAFlags(AppFlags flags) { AFlags |= flags; }
    void AddVFlags(ViewFlags flags) { VFlags |= flags; }

    void RemoveEFlags(EnvFlags flags) { EFlags &= ~flags; }
    void RemoveAFlags(AppFlags flags) { AFlags &= ~flags; }
    void RemoveVFlags(ViewFlags flags) { VFlags &= ~flags; }

    bool ContainsEFlags(EnvFlags flags) { return EFlags & flags; }
    bool ContainsAFlags(AppFlags flags) { return AFlags & flags; }
    bool ContainsVFlags(ViewFlags flags) { return VFlags & flags; }
};
