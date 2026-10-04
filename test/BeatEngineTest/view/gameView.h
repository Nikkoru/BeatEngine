#pragma once

#include "BeatEngine/Base/View.h"
#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/Graphics/TextElement.hpp"
#include "BeatEngine/View/ViewLayerStack.h"
#include "BeatEngine/Graphics/Renderer.h"

class GameView : public Base::View {
private:
    ViewLayerStack m_LayerStack;
    TextElement m_Text{};
public:
    GameView(AppContext* context, AppState* state);
public:
    void Init() override;
    void OnDraw(Renderer* const mgr) override;
    void OnEvent(const Optional<Base::Event> event) override;
    void OnUpdate(float dt) override;
    void OnExit() override;
};
