#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Manager/AudioManager.h"

void AppState::PrepareManagers(AppContext* context) {
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

ViewManager& AppState::GetViewMgr() {
    return ViewMgr;
}
SystemManager& AppState::GetSystemMgr() {
    return SystemMgr;
}
AssetManager& AppState::GetAssetMgr() {
    return AssetMgr;
}
SettingsManager& AppState::GetSettingsMgr() {
    return SettingsMgr;
}
UIManager& AppState::GetUIMgr() {
    return UIMgr;
}
AudioManager& AppState::GetAudioMgr() {
    return AudioMgr;
}
GraphicsManager& AppState::GetGraphicsMgr() {
    return GraphicsMgr;
}
