#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/View/ViewLayerStack.h"
class GameView : public Base::View {
private:
    ViewLayerStack m_LayerStack;
public:
    GameView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr);
public:
    void OnDraw(GraphicsManager* window) override;
    void OnEvent(const std::optional<Base::Event> event) override;
    void OnUpdate(float dt) override;
    void OnExit() override;
};
