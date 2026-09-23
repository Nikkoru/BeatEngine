#pragma once

#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/View/ViewLayer.h"
#include <memory>
#include <optional>

class GameLayer : public ViewLayer {
public:
    GameLayer() :
        ViewLayer(typeid(GameLayer), nullptr, nullptr) {}

    GameLayer(std::shared_ptr<AppContext> context, std::shared_ptr<AppState> state);
public:
    void Init() override {}

    void OnUpdate(float dt) override { (void)dt; }
    void OnEvent(Optional<Base::Event> event) override { (void)event; }
    void OnDraw(GraphicsManager&, RenderState) override {}
};
