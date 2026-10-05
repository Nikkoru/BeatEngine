#pragma once

#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/ViewManager.h"
#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/AudioManager.h"

class AppContext;
class AppState {
private:
    ViewManager ViewMgr{};
	SystemManager SystemMgr{};
	AssetManager AssetMgr{};
	SettingsManager SettingsMgr{};
	AudioManager AudioMgr{};
	UIManager UIMgr{};
public:
    AppState() = default;
    void PrepareManagers(AppContext* context);
public:
    ViewManager& GetViewMgr();
    SystemManager& GetSystemMgr();
    AssetManager& GetAssetMgr();
    SettingsManager& GetSettingsMgr();
    AudioManager& GetAudioMgr();
    UIManager& GetUIMgr();
};
