#include "BeatEngine/Game.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include "BeatEngine/Logger.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"
#include "BeatEngine/Settings/GameSettings.h"

#include "BeatEngine/Signals/ViewSignals.h"

Game::Game() {
	InitSettings();
	InitAudio();
	InitAssets();
	InitSystems();
	InitWindow();
	InitViews();
	InitUI();
	InitKeybinds();
	SubscribeToGameEvent();
	SubscribeToGameSignals();
}

Game::~Game() {
	delete m_ViewMgr;
	delete m_SystemMgr;
	delete m_AssetMgr;
	delete m_AudioMgr;

	delete m_Window;

	ImGui::SFML::Shutdown();
}

void Game::Run() {
	Logger::GetInstance()->AddInfo("Game started!", typeid(Game));

	if (!m_ViewMgr->HasActiveViews())
		m_ViewMgr->Push(m_ViewMgr->MainView);


	if (!m_Preloaded) {
		m_SettingsMgr->ReadConfig(m_SettingsPath);
		ApplyBaseSettings();
	}

	while (this->m_Window->isOpen()) {
		while (const auto event = this->m_Window->pollEvent()) {
			if (m_UseImGui)
				ImGui::SFML::ProcessEvent(*m_Window, *event);

			if (event->is<sf::Event::Closed>())
				this->m_Window->close();
			if (auto data = event->getIf<sf::Event::Resized>()) {
				m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(data->size.x), static_cast<float>(data->size.y) }));
				m_Window->setView(m_View);
			}

			if (!this->m_ViewMgr->OnSFMLEvent(event)) {
				this->m_Window->close();
				break;
			}

			m_GlobalLayers.OnSFMLEvent(event);
		}
		this->Update();
		this->Draw();
		this->Display();
	}
}

void Game::UseImGui(bool show) {
	m_UseImGui = show;
}

sf::Window* Game::GetWindow() {
	return m_Window;
}

void Game::SetWindowSize(sf::Vector2u size) {
	m_Window->setSize(size);
}

void Game::SetWindowTitle(std::string title) {
	m_Window->setTitle(title);
}

void Game::PreloadSettings() {
	m_Preloaded = true;

	m_SettingsMgr->ReadConfig(m_SettingsPath);
}

void Game::SaveSettings() {
	m_SettingsMgr->WriteConfig(m_SettingsPath);
}

void Game::SetConfigPath(std::filesystem::path path) {
	this->m_SettingsPath = path;
}

void Game::LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets) {
	if (globalAssets.empty())
		return;
	for (auto& [type, vecPath] : globalAssets) {
		switch (type) {
		case AssetType::Texture:
			for (auto& path : vecPath)
				m_AssetMgr->Load<Texture>(path);
			break;
		case AssetType::AudioStream:
			for (auto& path : vecPath)
				m_AssetMgr->Load<AudioStream>(path);
			break;
		case AssetType::Sound:
			for (auto& path : vecPath)
				m_AssetMgr->Load<Sound>(path);
			break;
		case AssetType::Font:
			for (auto& path : vecPath)
				m_AssetMgr->Load<Font>(path);
		}
	}
}

void Game::Display() {
	if (m_UseImGui)
		ImGui::SFML::Render(*m_Window);

	m_Window->display();
}

void Game::Draw() {
	m_Window->clear();

	// m_AnimationMgr->DrawActiveAnimation(m_DeltaClock);
	if (!m_ViewMgr->OnDraw(m_Window))
		m_Window->close();

	m_Window->draw(m_GlobalLayers);
}

void Game::Update() {
	auto sfDelta = m_Clock.restart();
	auto deltaTime = sfDelta.asSeconds();

	if (m_UseImGui)
		ImGui::SFML::Update(*m_Window, sfDelta);

	if (!this->m_ViewMgr->OnUpdate(deltaTime)) {
		this->m_Window->close();
		return;
	}
	this->m_SystemMgr->Update(deltaTime);

	m_GlobalLayers.OnUpdate(deltaTime);
}

void Game::ApplyBaseSettings() {
	auto gameSettings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));

	m_Window->setFramerateLimit(gameSettings->FpsLimit);
	m_Window->setSize(gameSettings->WindowSize);

	if (gameSettings->WindowPosition != sf::Vector2i(-1, -1))
		m_Window->setPosition(gameSettings->WindowPosition);
}

void Game::InitSettings() {
	Logger::GetInstance()->AddInfo("Initializing settings...", typeid(Game));
	this->m_SettingsMgr = new SettingsManager;

	m_SettingsMgr->RegisterSettingsData<GameSettings>();
}

void Game::InitUI() {
	Logger::GetInstance()->AddInfo("Initializing UI...", typeid(Game));
	this->m_UIMgr = new UIManager;
}

void Game::InitAudio() {
	Logger::GetInstance()->AddInfo("Initializing audio...", typeid(Game));
	this->m_AudioMgr = new AudioManager;
}

void Game::InitViews() {
	Logger::GetInstance()->AddInfo("Initializing views...", typeid(Game));
	this->m_ViewMgr = new ViewManager;
	this->m_ViewMgr->SetGlobalAssetManager(m_AssetMgr);
}

void Game::InitSystems() {
	Logger::GetInstance()->AddInfo("Initializing systems...", typeid(Game));
	this->m_SystemMgr = new SystemManager;
}

void Game::InitAssets() {
	Logger::GetInstance()->AddInfo("Initializing assets...", typeid(Game));
	this->m_AssetMgr = new AssetManager;
}

void Game::InitWindow() {
	Logger::GetInstance()->AddInfo("Initializing window...", typeid(Game));

	auto settings = m_SettingsMgr->GetSettings(typeid(GameSettings));

	auto gameSettings = std::static_pointer_cast<GameSettings>(settings);

	this->m_Window = new sf::RenderWindow(sf::VideoMode(gameSettings->WindowSize), "BeatEngine Game");
	this->m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(gameSettings->WindowSize.x), static_cast<float>(gameSettings->WindowSize.y) }));
	this->m_Window->setFramerateLimit(gameSettings->FpsLimit);
	this->m_Window->setView(m_View);

	if (!ImGui::SFML::Init(*m_Window)) {
        m_Window->close();
    }
}

void Game::InitKeybinds() {
	Logger::GetInstance()->AddInfo("Initializing keybinds... (not really)", typeid(Game));
}

void Game::SubscribeToGameEvent() {
	Logger::GetInstance()->AddInfo("Subscribing to game events...", typeid(Game));
}

void Game::SubscribeToGameSignals() {
	Logger::GetInstance()->AddInfo("Subscribing to game signals...", typeid(Game));

	SignalManager::GetInstance()->RegisterCallback<ViewAddGlobalLayerSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
		auto signal = std::static_pointer_cast<ViewAddGlobalLayerSignal>(sig);
		this->m_GlobalLayers.AttachLayer(signal->Layer);

		signal->Layer = nullptr;
	});
}
