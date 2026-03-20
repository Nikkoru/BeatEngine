#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/View/ViewLayerStack.h"
#include <memory>
class GameView : public Base::View {
private:
    ViewLayerStack m_LayerStack;
public:
    GameView(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state);
public:
    void OnDraw(GraphicsManager* window) override;
    void OnEvent(const std::optional<Base::Event> event) override;
    void OnUpdate(float dt) override;
    void OnExit() override;
};
