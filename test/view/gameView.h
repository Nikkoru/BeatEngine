#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/View/ViewLayerStack.h"
#include <SFML/Graphics/RenderWindow.hpp>
class GameView : public Base::View {
private:
    ViewLayerStack m_LayerStack;
public:
    GameView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr);
public:
    void OnDraw(sf::RenderWindow* window) override;
    void OnSFMLEvent(const std::optional<sf::Event> event) override;
    void OnUpdate(float dt) override;
    void OnExit() override;
};
