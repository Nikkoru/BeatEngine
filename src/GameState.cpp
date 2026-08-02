#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/AudioManager.h"

void GameState::PrepareManagers(GameContext* context) {
    ViewMgr.SetContext(context);
    ViewMgr.SetState(this);
    SystemMgr.SetContext(context);
    SystemMgr.SetState(this);
    AssetMgr.SetContext(context);
    AssetMgr.SetState(this);
    SettingsMgr.SetContext(context);
    SettingsMgr.SetState(this);
    UIMgr.SetContext(context);
    UIMgr.SetState(this);
    AudioMgr.SetContext(context);
    AudioMgr.SetState(this);

    GraphicsMgr = GraphicsManager(context, this);
}

ViewManager& GameState::GetViewMgr() {
    return ViewMgr;
}
SystemManager& GameState::GetSystemMgr() {
    return SystemMgr;
}
AssetManager& GameState::GetAssetMgr() {
    return AssetMgr;
}
SettingsManager& GameState::GetSettingsMgr() {
    return SettingsMgr;
}
UIManager& GameState::GetUIMgr() {
    return UIMgr;
}
AudioManager& GameState::GetAudioMgr() {
    return AudioMgr;
}
GraphicsManager& GameState::GetGraphicsMgr() {
    return GraphicsMgr;
}
