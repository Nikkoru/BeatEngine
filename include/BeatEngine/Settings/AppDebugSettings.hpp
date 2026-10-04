#pragma once

#include "BeatEngine/Base/Settings.h"

class AppDebugSettings : public Base::Settings {
public:
    bool DrawAudioMgr = false;
    bool DrawAssetMgr = false;
    bool DrawEntityMgr = false;
    bool DrawEventMgr = false;
    bool DrawGraphicsMgr = false;
    bool DrawSettingsMgr = false;
    bool DrawSignalMgr = false;
    bool DrawSystemMgr = false;
    bool DrawUIMgr = false;
    bool DrawViewMgr = false;
public:
    AppDebugSettings() : Base::Settings(typeid(AppDebugSettings), "[Debug]") {}
    ~AppDebugSettings() override = default;

    void Read(const char *line) override; 
    std::string Write() override;
    void SetDefaults() override;
};
