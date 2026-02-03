#include "gameView.h"
#include "../layer/GameLayer.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/ViewSignals.h"
#include <SFML/Window/Keyboard.hpp>
#include <memory>

GameView::GameView(GameContext* context, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, UIManager* uiMgr) :
Base::View(typeid(GameView), context, assetMgr, settingsMgr, audioMgr, uiMgr) {
    auto layer = b_mLayerStack.AttachLayer<GameLayer>();
    layer->SetGameContext(context);
    layer->SetAssetManager(assetMgr);
    layer->SetSettingsManager(settingsMgr);
    layer->SetAudioManager(audioMgr);
    layer->SetUIManager(uiMgr);
}

void GameView::OnDraw(sf::RenderWindow* window) {

}
void GameView::OnSFMLEvent(const std::optional<sf::Event> event) {
    if (auto data = event->getIf<sf::Event::KeyPressed>()) {
        if (data->scancode == sf::Keyboard::Scan::Escape)
            SignalManager::GetInstance()->Send(std::make_shared<ViewPopSignal>());
    }

}
void GameView::OnUpdate(float dt) {

}
void GameView::OnExit() {

}
