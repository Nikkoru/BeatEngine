#pragma once
#include "BeatEngine/Enum/AppFlags.hpp"
#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include <string>

struct AppConfig {
    std::string appName;
    std::string configPath{};
    
    bool initImGui{ false };

    AppFlags initAFlags{};
    EnvFlags initEFlags{};
    ViewFlags initVFlags{};
};
