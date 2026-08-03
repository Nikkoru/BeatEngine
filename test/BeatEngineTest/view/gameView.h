#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/View/ViewLayerStack.h"
class GameView : public Base::View {
private:
    ViewLayerStack m_LayerStack;
public:
    GameView(GameContext* context, GameState* state);
public:
    void Init() override;
    void OnDraw(GraphicsManager& mgr) override;
    void OnEvent(const Optional<Base::Event> event) override;
    void OnUpdate(float dt) override;
    void OnExit() override;
};
