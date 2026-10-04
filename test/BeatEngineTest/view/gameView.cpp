#include "gameView.h"
#include "../layer/GameLayer.h"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Graphics/Renderer.h"

// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/ViewSignals.h"

GameView::GameView(AppContext* context, AppState* state) :
Base::View(typeid(GameView), context, state) {
    auto layer = b_mLayerStack.AttachLayer<GameLayer>();
    layer->SetAppContext(context);
    layer->SetAppState(state);
}

void GameView::Init() {
    auto font = b_mState->GetAssetMgr().Get<Font>("main-font");
    m_Text = TextElement{ font, "GameView", 30 };
    m_Text.SetPosition(Vector2f{ Vector2u{ b_mContext->WindowSize.X / 2, b_mContext->WindowSize.Y / 2 } });
}

void GameView::OnDraw(Renderer* const mgr) {
    m_Text.Draw(mgr);

}
void GameView::OnEvent(const Optional<Base::Event>) {
    // if (auto data = event->getIf<sf::Event::KeyPressed>()) {
    //     if (data->scancode == sf::Keyboard::Scan::Escape)
    //         SignalManager::GetInstance()->Send(std::make_shared<ViewPopSignal>());
    // }
}
void GameView::OnUpdate(float) {

}
void GameView::OnExit() {

}
