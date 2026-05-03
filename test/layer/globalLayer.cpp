#include "globalLayer.h"
// #include "BeatEngine/Enum/GameFlags.h"
// #include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Manager/AssetManager.h"
#include "BeatEngine/Settings/GameSettings.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/SettingsSignals.h"
#include "BeatEngine/GameState.h"

#include <BeatEngine/Manager/SignalManager.h>
#include <BeatEngine/Signals/ViewSignals.h>

#include <BeatEngine/UI/Elements/Button.h>

#include <format>
// #include <imgui.h>
#include <memory>

GlobalTestLayerUI::GlobalTestLayerUI() : GlobalTestLayerUI(nullptr, nullptr) {
}

GlobalTestLayerUI::GlobalTestLayerUI(GameContext* context, GameState* state) : ViewLayer(typeid(GlobalTestLayerUI), context, state) {
	m_HUD = state->GetUIMgr().AddLayer("GlobalTestLayerUI", true);

	// m_Font = assetMgr->Get<Font>("main-font").Get();
    auto windowSize = m_Context->WindowSize;

	auto root = m_HUD->SetRootElement<UI::Button>();
    
    auto exitBtn = root->AddChild<UI::Button>("exitBtn");
    auto toggleVSyncBtn = root->AddChild<UI::Button>("vSyncToggle");
    auto toggleFullscreenBtn = root->AddChild<UI::Button>("fullscreenToggle");
    auto settings = std::static_pointer_cast<GameSettings>(m_State->GetSettingsMgr().GetSettings(typeid(GameSettings)));

	// root->SetFont(*m_Font);
	root->SetSize({80, 30});
    root->SetText("120 FPS");

    root->SetOnLClick([this, settings]() {
        settings->FpsLimit = 120;

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });

    // exitBtn->SetFont(*m_Font);
    exitBtn->SetSize({ 80, 30 });
    exitBtn->SetText("Exit");

    exitBtn->SetOnLClick([]() {
        SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
    });

    // toggleVSyncBtn->SetFont(*m_Font);
    toggleVSyncBtn->SetSize({ 110, 30 });
    if (settings->VSync)
        toggleVSyncBtn->SetText("VSync On");
    else
        toggleVSyncBtn->SetText("VSync Off");

    toggleVSyncBtn->SetOnLClick([toggleVSyncBtn, settings, this]() {
        
        settings->VSync = !settings->VSync;
        if (settings->VSync) 
            toggleVSyncBtn->SetText("VSync On"); 
        else
            toggleVSyncBtn->SetText("VSync Off");

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });

    // toggleFullscreenBtn->SetFont(*m_Font);
    toggleFullscreenBtn->SetSize({ 110, 30 });
    if (settings->WindowFullScreen)
        toggleFullscreenBtn->SetText("In fullscreen");
    else
        toggleFullscreenBtn->SetText("In window");
    toggleFullscreenBtn->SetOnLClick([toggleFullscreenBtn, settings, this]() {

        settings->WindowFullScreen = !settings->WindowFullScreen;

        if (settings->WindowFullScreen)
            toggleFullscreenBtn->SetText("In fullscreen");
        else
            toggleFullscreenBtn->SetText("In windows");

        SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    });

    UpdatePositions();
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

void GlobalTestLayerUI::OnEvent(std::optional<Base::Event> event) {
	// m_HUD->OnSFMLEvent(event);
	//
 //    if (auto data = event->getIf<sf::Event::KeyPressed>()) {
 //        if (data->scancode == sf::Keyboard::Scan::Grave) {
 //            ToggleImGuiDrawing();
 //        }
 //        else if (data->scancode == sf::Keyboard::Scan::F1) {
 //            m_HUD->SetVisible(!m_HUD->IsVisible());
 //        }
 //    }
 //    else if (event->is<sf::Event::Resized>()) {
 //        UpdatePositions();
 //    }
}

void GlobalTestLayerUI::OnDraw() {
 //    auto font = m_Font->GetSFMLFont();
	//
	// auto fpsText = sf::Text(*font, m_FPSText, 15);
	// fpsText.setPosition({ 0, 0 });
 //    if (std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)))->VSync)
 //        fpsText.setFillColor(sf::Color::Yellow);
 //    else
 //        fpsText.setFillColor(sf::Color::White);
	//
 //    auto deltaText = sf::Text(*font, m_DeltaText, 15);
 //    deltaText.setPosition({0, fpsText.getGlobalBounds().size.y + 1});
 //    deltaText.setFillColor(sf::Color::White);
 //    
 //    auto rect = sf::RectangleShape();
 //    rect.setFillColor(sf::Color(50, 50, 50, 255));
 //    rect.setSize({ 55, 30 }); 
	//
 //    target.draw(rect);
	//
	// target.draw(fpsText);
	// target.draw(deltaText);
	// target.draw(*m_HUD);
    DrawImGuiDebug();
}

void GlobalTestLayerUI::ToggleImGuiDrawing() {
    m_DrawDebug = !m_DrawDebug; 
    SignalManager::GetInstance()->Send(std::make_shared<GameToggleDrawingDebugInfo>());
}

void GlobalTestLayerUI::UpdatePositions() {
    auto windowSize = m_Context->WindowSize;

    auto root = m_HUD->GetRootElement<UI::Button>();
    
    auto exitBtn = root->GetChild<UI::Button>("exitBtn");
    auto toggleVSyncBtn = root->GetChild<UI::Button>("vSyncToggle");
    auto toggleFullscreenBtn = root->GetChild<UI::Button>("fullscreenToggle");

    root->SetPosition({ 90, 100 });
    exitBtn->SetPosition({ 5, (windowSize.Y - 5) - exitBtn->GetSize().Y });
    toggleVSyncBtn->SetPosition({ (windowSize.X - 5) - toggleVSyncBtn->GetSize().X, 100 });
    toggleFullscreenBtn->SetPosition({(windowSize.X - 5) - toggleFullscreenBtn->GetSize().X, 135 });}

void GlobalTestLayerUI::DrawImGuiDebug() const {
    // auto text = std::static_pointer_cast<GameSettings>(m_State->GetSettingsMgr().GetSettings(typeid(GameSettings)))->WindowFullScreen ? "In Fullscreen" : "In Window";
    //
    // ImGui::Begin("wa");
    // ImGui::Text("wa");
    // if (ImGui::Button(text)) {
    //     auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
    //     settings->WindowFullScreen = !settings->WindowFullScreen;
    //
    //     SignalManager::GetInstance()->Send(std::make_shared<SetSettingsSignal>(typeid(GameSettings), settings));
    // }
    // ImGui::End();

    m_State->GetViewMgr().ShowImGuiDebugWindow();
}
