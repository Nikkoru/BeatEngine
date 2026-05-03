#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/AudioManager.h"

void GameState::CreateManagers(GameContext* context) {
    ViewMgr = ViewManager(context, this);
    SystemMgr = SystemManager(context, this);
    AssetMgr = AssetManager(context, this);
    SettingsMgr = SettingsManager(context, this);
    UIMgr = UIManager(context, this);
    AudioMgr = AudioManager(context, this);
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
