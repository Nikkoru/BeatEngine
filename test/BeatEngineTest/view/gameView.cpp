#include "gameView.h"
#include "../layer/GameLayer.h"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/Manager/GraphicsManager.h"


// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/ViewSignals.h"

GameView::GameView(AppContext* context, AppState* state) :
Base::View(typeid(GameView), context, state) {
    auto layer = b_mLayerStack.AttachLayer<GameLayer>();
    layer->SetAppContext(context);
    layer->SetAppState(state);
}

void GameView::Init() {

}

void GameView::OnDraw(GraphicsManager&) {

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
