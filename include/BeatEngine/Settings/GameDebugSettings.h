#pragma once

#include "BeatEngine/Base/Settings.h"

class GameDebugSettings : public Base::Settings {
public:
    
public:
    GameDebugSettings() : Base::Settings(typeid(GameDebugSettings), "[Debug]") {}
    ~GameDebugSettings() override = default;

    void Read(const char *line) override; 
    std::string Write() override;
    void SetDefaults() override;
};
