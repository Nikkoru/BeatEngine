#include "globalLayer.h"
#include "BeatEngine/Settings/GameSettings.h"
#include "BeatEngine/Signals/SettingsSignals.h"

#include <BeatEngine/Manager/SignalManager.h>
#include <BeatEngine/Signals/ViewSignals.h>

#include <BeatEngine/UI/Elements/Button.h>

#include <format>
#include <memory>

GlobalTestLayerUI::GlobalTestLayerUI() : GlobalTestLayerUI(nullptr, nullptr) {
}

GlobalTestLayerUI::GlobalTestLayerUI(UIManager* uiMgr, AssetManager* assetMgr) : ViewLayer(typeid(GlobalTestLayerUI), uiMgr, assetMgr) {
	m_HUD = uiMgr->AddLayer("GlobalTestLayerUI", true);

	auto font = assetMgr->Get<Font>("main-font").Get();
	auto root = m_HUD->SetRootElement<UI::Button>();

	root->SetFont(*font);
	root->SetSize({80, 30});
	root->SetPosition({ 90, 100 });

    root->SetOnLClick([this]() {
        auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
        settings->FpsLimit = 120;

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });
}

void GlobalTestLayerUI::OnUpdate(float dt) {
	m_HUD->Update(dt);
}

void GlobalTestLayerUI::OnAttach() {
}

void GlobalTestLayerUI::OnDetach() {
}

void GlobalTestLayerUI::OnSFMLEvent(std::optional<sf::Event> event) {
	m_HUD->OnSFMLEvent(event);
}

void GlobalTestLayerUI::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	auto text = sf::Text(*m_AssetMgr->Get<Font>("main-font").Get()->GetSFMLFont(), m_Text, 15);
	text.setPosition({ 100, 100 });
	target.draw(text);
	target.draw(*m_HUD);
}
