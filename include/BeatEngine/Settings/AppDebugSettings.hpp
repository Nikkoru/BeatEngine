#pragma once

#include "BeatEngine/Base/Settings.h"

class AppDebugSettings : public Base::Settings {
public:
    
public:
    AppDebugSettings() : Base::Settings(typeid(AppDebugSettings), "[Debug]") {}
    ~AppDebugSettings() override = default;

    void Read(const char *line) override; 
    std::string Write() override;
    void SetDefaults() override;
};
