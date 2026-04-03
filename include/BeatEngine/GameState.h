#pragma once

#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/ViewManager.h"
#include "BeatEngine/Manager/SystemManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/AudioManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"

class GameContext;
class GameState {
private:
    ViewManager ViewMgr;
	SystemManager SystemMgr;
	AssetManager AssetMgr;
	SettingsManager SettingsMgr;
	AudioManager AudioMgr;
	UIManager UIMgr;
    GraphicsManager GraphicsMgr;
public:
    GameState() = default;
    void CreateManagers(GameContext* context);
public:
    ViewManager& GetViewMgr();
    SystemManager& GetSystemMgr();
    AssetManager& GetAssetMgr();
    SettingsManager& GetSettingsMgr();
    AudioManager& GetAudioMgr();
    UIManager& GetUIMgr();
    GraphicsManager& GetGraphicsMgr();
};
