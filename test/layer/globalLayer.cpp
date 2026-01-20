#include "globalLayer.h"
#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Settings/GameSettings.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/SettingsSignals.h"

#include <BeatEngine/Manager/SignalManager.h>
#include <BeatEngine/Signals/ViewSignals.h>

#include <BeatEngine/UI/Elements/Button.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <format>
#include <imgui.h>
#include <memory>

GlobalTestLayerUI::GlobalTestLayerUI() : GlobalTestLayerUI(nullptr, nullptr, nullptr, nullptr, nullptr) {
}

GlobalTestLayerUI::GlobalTestLayerUI(UIManager* uiMgr, AssetManager* assetMgr, SettingsManager* settingsMgr, AudioManager* audioMgr, SystemManager* systemMgr) : ViewLayer(typeid(GlobalTestLayerUI), uiMgr, assetMgr, settingsMgr, audioMgr, systemMgr) {
	m_HUD = uiMgr->AddLayer("GlobalTestLayerUI", true);

	m_Font = assetMgr->Get<Font>("main-font").Get();
	auto root = m_HUD->SetRootElement<UI::Button>();
    
    auto exitBtn = root->AddChild<UI::Button>("exitBtn");
    auto toggleVSyncBtn = root->AddChild<UI::Button>("vSyncToggle");
    auto toggleFullscreenBtn = root->AddChild<UI::Button>("fullscreenToggle");

	root->SetFont(*m_Font);
	root->SetSize({80, 30});
	root->SetPosition({ 90, 100 });
    root->SetText("120 FPS");

    root->SetOnLClick([this]() {
        auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
        settings->FpsLimit = 120;

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });

    exitBtn->SetFont(*m_Font);
    exitBtn->SetSize({ 80, 30 });
    exitBtn->SetPosition({ 5, 600 - 5 - 30 });
    exitBtn->SetText("Exit");

    exitBtn->SetOnLClick([]() {
        SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
    });

    toggleVSyncBtn->SetFont(*m_Font);
    toggleVSyncBtn->SetSize({ 110, 30 });
    toggleVSyncBtn->SetPosition({ 795 - toggleVSyncBtn->GetSize().x, 100 });
    if (std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)))->VSync)
        toggleVSyncBtn->SetText("VSync On");
    else
        toggleVSyncBtn->SetText("VSync Off");

    toggleVSyncBtn->SetOnLClick([toggleVSyncBtn, this]() {
        auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
        
        settings->VSync = !settings->VSync;
        if (settings->VSync) 
            toggleVSyncBtn->SetText("VSync On"); 
        else
            toggleVSyncBtn->SetText("VSync Off");

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });

    toggleFullscreenBtn->SetFont(*m_Font);
    toggleFullscreenBtn->SetSize({ 110, 30 });
    toggleFullscreenBtn->SetPosition({795 - toggleFullscreenBtn->GetSize().x, 135 });
    if (std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)))->WindowFullScreen)
        toggleFullscreenBtn->SetText("In fullscreen");
    else
        toggleFullscreenBtn->SetText("In window");
    toggleFullscreenBtn->SetOnLClick([toggleFullscreenBtn, this]() {
        auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));

        settings->WindowFullScreen = !settings->WindowFullScreen;

        if (settings->WindowFullScreen)
            toggleFullscreenBtn->SetText("In fullscreen");
        else
            toggleFullscreenBtn->SetText("In windows");

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });
}

void GlobalTestLayerUI::OnUpdate(float dt) {
    m_FPSText = std::format("FPS {:.2f}", 1 / dt);
    m_DeltaText = std::format("DT {:.3f}", dt);
	m_HUD->Update(dt);
    m_HUD->GetRootElement<UI::Button>()->GetChild<UI::Button>("exitBtn")->Update(dt);
    m_HUD->GetRootElement<UI::Button>()->GetChild<UI::Button>("vSyncToggle")->Update(dt);
}

void GlobalTestLayerUI::OnAttach() {
}

void GlobalTestLayerUI::OnDetach() {
}

void GlobalTestLayerUI::OnSFMLEvent(std::optional<sf::Event> event) {
	m_HUD->OnSFMLEvent(event);

    if (auto data = event->getIf<sf::Event::KeyPressed>()) {
        if (data->scancode == sf::Keyboard::Scan::Grave) {
            ToggleImGuiDrawing();
        }
    }
}

void GlobalTestLayerUI::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto font = m_Font->GetSFMLFont();

	auto fpsText = sf::Text(*font, m_FPSText, 15);
	fpsText.setPosition({ 0, 0 });
    if (std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)))->VSync)
        fpsText.setFillColor(sf::Color::Yellow);
    else
        fpsText.setFillColor(sf::Color::White);

    auto deltaText = sf::Text(*font, m_DeltaText, 15);
    deltaText.setPosition({0, fpsText.getGlobalBounds().size.y + 1});
    deltaText.setFillColor(sf::Color::White);
    
    auto rect = sf::RectangleShape();
    rect.setFillColor(sf::Color(50, 50, 50, 255));
    rect.setSize({ 55, 30 }); 

    target.draw(rect);

	target.draw(fpsText);
	target.draw(deltaText);
	target.draw(*m_HUD);

    DrawImGuiDebug();
}

void GlobalTestLayerUI::ToggleImGuiDrawing() {
   m_DrawDebug = !m_DrawDebug; 
}

void GlobalTestLayerUI::DrawImGuiDebug() const {
    ImGui::Begin("wa");
    ImGui::Text("wa");
    ImGui::End();

    m_SettingsMgr->DrawImGuiDebug();
    m_UIMgr->DrawImGuiDebug();
    m_AssetMgr->DrawImGuiDebug();
    // m_AudioMgr->DrawImGuiDebug();
    m_SystemMgr->DrawImGuiDebug();
}
