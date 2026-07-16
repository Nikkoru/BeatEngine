#pragma once

#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/View/ViewLayer.h"
#include <memory>
#include <optional>

class GameLayer : public ViewLayer {
public:
    GameLayer() :
        ViewLayer(typeid(GameLayer), nullptr, nullptr) {}

    GameLayer(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state);
public:
    void Init() override {}

    void OnUpdate(float dt) override { (void)dt; }
    void OnEvent(Optional<Base::Event> event) override { (void)event; }
    void OnDraw(GraphicsManager&) override {}
};
