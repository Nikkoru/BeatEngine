#include "gameView.h"
#include "../layer/GameLayer.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <memory>
// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/ViewSignals.h"

GameView::GameView(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state) :
Base::View(typeid(GameView), context, state) {
    auto layer = b_mLayerStack.AttachLayer<GameLayer>();
    layer->SetGameContext(context);
    layer->SetGameState(state);
}

void GameView::OnDraw(GraphicsManager* window) {

}
void GameView::OnEvent(const std::optional<Base::Event> event) {
    // if (auto data = event->getIf<sf::Event::KeyPressed>()) {
    //     if (data->scancode == sf::Keyboard::Scan::Escape)
    //         SignalManager::GetInstance()->Send(std::make_shared<ViewPopSignal>());
    // }
}
void GameView::OnUpdate(float dt) {

}
void GameView::OnExit() {

}
