#pragma once

#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/View/ViewLayer.h"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>

class GameLayer : public ViewLayer {
public:
    GameLayer() :
    ViewLayer(typeid(GameLayer),
              nullptr,
              nullptr,
              nullptr,
              nullptr,
              nullptr,
              nullptr)
{}
    GameLayer(GameContext* context, 
              UIManager* uiMgr, 
              AssetManager* assetMgr,
              SettingsManager* settingsMgr,
              AudioManager* audioMgr,
              SystemManager* systemMgr);
public:
    void OnUpdate(float dt) override {}
    void OnSFMLEvent(std::optional<sf::Event> event) override {}

    void draw(sf::RenderTarget&, sf::RenderStates) const override {}
};
