#include "gameView.h"
#include "../layer/GameLayer.h"
#include "BeatEngine/Manager/GraphicsManager.h"
// #include "BeatEngine/Manager/SignalManager.h"
// #include "BeatEngine/Signals/ViewSignals.h"

GameView::GameView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr) :
Base::View(typeid(GameView), context, assetMgr, settingsMgr, audioMgr, uiMgr) {
    auto layer = b_mLayerStack.AttachLayer<GameLayer>();
    layer->SetGameContext(context);
    layer->SetAssetManager(assetMgr);
    layer->SetSettingsManager(settingsMgr);
    layer->SetAudioManager(audioMgr);
    layer->SetUIManager(uiMgr);
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
