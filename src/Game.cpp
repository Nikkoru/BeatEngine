#include "BeatEngine/Game.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>
#include <memory>

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Settings/GameSettings.h"

#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Events/GameEvent.h"

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
	delete m_AudioMgr;
	delete m_SystemMgr;
	delete m_AssetMgr;

    delete m_Window;
	
	ImGui::SFML::Shutdown();
}

void Game::Run() {
    m_Running = true;
	Logger::AddInfo(typeid(Game), "Game started!");

	if (!m_ViewMgr->HasActiveViews())
		m_ViewMgr->Push(m_ViewMgr->MainView);


	if (m_Flags & GameFlags_Preload) {
		m_SettingsMgr->ReadConfig(m_SettingsPath);
		ApplyBaseSettings();
	}

	while (this->m_Window->isOpen() && m_Running) {
		while (const auto event = this->m_Window->pollEvent()) {
			if (m_Flags & GameFlags_ImGui)
				ImGui::SFML::ProcessEvent(*m_Window, *event);
            if ((event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>()) && (m_Flags & GameFlags_DisableKeyPressEvents))
                continue;
			if (event->is<sf::Event::Closed>()) {
                EventManager::GetInstance()->Send(std::make_shared<GameExitingEvent>());
                m_Running = false;
            }
			if (auto data = event->getIf<sf::Event::Resized>()) {
				m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(data->size.x), static_cast<float>(data->size.y) }));
				m_Window->setView(m_View);

                EventManager::GetInstance()->Send(std::make_shared<GameResized>(data->size));
			}
            if (m_Window->isOpen() && m_Running) {
		    	m_GlobalLayers.OnSFMLEvent(event);
            }
            else
                break;

            if (m_Window->isOpen() && m_Running) {
			    if (!this->m_ViewMgr->OnSFMLEvent(event)) {
				    this->m_Window->close();
				    break;
			    }
            }
            else {
                m_Window->close();
                break;
            }
		}
        if (m_Window->isOpen() && m_Running) {
		    this->Update();
		    this->Draw();
		    this->Display();
        }
        else
            break;
	}
}

void Game::UseImGui(bool show) {
    if (show)
        m_Flags |= GameFlags_ImGui;
    else
        m_Flags &= ~GameFlags_ImGui;
}

void Game::UseImGuiDocking(bool docking) {
    if (docking)
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    else
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
}

sf::Window* Game::GetWindow() {
	return m_Window;
}

SettingsManager* Game::GetSettingsManager() {
    return m_SettingsMgr;
}

void Game::SetWindowSize(sf::Vector2u size) {
    auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
    settings->WindowSize = size;

	m_Window->setSize(size);
}

void Game::SetWindowTitle(std::string title) {
	m_Window->setTitle(title);
}

void Game::PreloadSettings() {
    m_Flags |= GameFlags_Preload;

	m_SettingsMgr->ReadConfig(m_SettingsPath);
}

void Game::SaveSettings() {
	m_SettingsMgr->WriteConfig(m_SettingsPath);
}

void Game::SetConfigPath(std::filesystem::path path) {
	this->m_SettingsPath = path;
}

void Game::SetFlags(GameFlags flags) {
    this->m_Flags |= flags;
}

void Game::RemoveFlags(GameFlags flags) {
    this->m_Flags &= ~flags;
}

void Game::DrawImGuiDebug() {
    ImGui::Begin("BeatEngine Game Debug Window");
    bool imguiToggle = m_Flags & GameFlags_ImGui;
    bool preloadToggle = m_Flags & GameFlags_Preload;
    bool fullscreenToggle = m_Flags & GameFlags_Fullscreen;
    bool cursorChangedToggle = m_Flags & GameFlags_CursorChanged;
    bool disableKeysToggle = m_Flags & GameFlags_DisableKeyPressEvents;
    bool drawDebugToggle = m_Flags & GameFlags_DrawDebugInfo;


    ImGui::Checkbox("GameFlags_ImGui", &imguiToggle);
    ImGui::Checkbox("GameFlags_Preload", &preloadToggle);
    ImGui::Checkbox("GameFlags_Fullscreen", &fullscreenToggle);
    ImGui::Checkbox("GameFlags_CursorChanged", &cursorChangedToggle);
    ImGui::Checkbox("GameFlags_DisableKeyPressEvents", &disableKeysToggle);
    ImGui::Checkbox("GameFlags_DrawDebugInfo", &drawDebugToggle);
    ImGui::End();
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
	if (m_Flags & GameFlags_ImGui)
		ImGui::SFML::Render(*m_Window);

	m_Window->display();
}

void Game::Draw() {
	m_Window->clear();

	// m_AnimationMgr->DrawActiveAnimation(m_DeltaClock);
	if (!m_ViewMgr->OnDraw(m_Window))
		m_Window->close();

	m_Window->draw(m_GlobalLayers);

    if ((m_Flags & GameFlags_DrawDebugInfo) && (m_Flags & GameFlags_ImGui))
        DrawImGuiDebug();
}

void Game::Update() {
    if (m_Flags & GameFlags_CursorChanged) {
        m_Window->setMouseCursor(m_Cursor);
        m_Flags &= ~GameFlags_CursorChanged;
    }

	auto sfDelta = m_Clock.restart();
	auto deltaTime = sfDelta.asSeconds();

	if (m_Flags & GameFlags_ImGui)
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
	Logger::AddInfo(typeid(Game), "Initializing settings...");
	this->m_SettingsMgr = new SettingsManager;

	m_SettingsMgr->RegisterSettingsData<GameSettings>();
    m_SettingsMgr->ReadConfig(m_SettingsPath);

}

void Game::InitUI() {
	Logger::AddInfo(typeid(Game), "Initializing UI...");
	this->m_UIMgr = new UIManager;
}

void Game::InitAudio() {
	Logger::AddInfo(typeid(Game), "Initializing audio...");
    Logger::AddWarning(typeid(Game), "AudioManager is in a broken state. It is not recomended to use it. Any call for this manager please comment it out as AudioManager is not initialized.");
#ifdef BEATENGINE_TEST
    Logger::AddInfo(typeid(Game), "But this a test build, so initilizing AudioManager...");
	this->m_AudioMgr = new AudioManager;
#endif
}

void Game::InitViews() {
	Logger::AddInfo(typeid(Game), "Initializing views...");
	this->m_ViewMgr = new ViewManager;
	this->m_ViewMgr->SetGlobalAssetManager(m_AssetMgr);
    this->m_ViewMgr->SetGlobalAudioManager(m_AudioMgr);
    this->m_ViewMgr->SetGlobalSettingsManager(m_SettingsMgr);
    this->m_ViewMgr->SetGlobalUIManager(m_UIMgr);
}

void Game::InitSystems() {
    Logger::AddInfo(typeid(Game), "Initializing systems...");
	this->m_SystemMgr = new SystemManager;
}

void Game::InitAssets() {
	Logger::AddInfo(typeid(Game), "Initializing assets...");
	this->m_AssetMgr = new AssetManager;
}

void Game::InitWindow() {
	Logger::AddInfo(typeid(Game), "Initializing window...");

	auto settings = m_SettingsMgr->GetSettings(typeid(GameSettings));

	auto gameSettings = std::static_pointer_cast<GameSettings>(settings);
	this->m_Window = new sf::RenderWindow(
	    sf::VideoMode(gameSettings->WindowSize), 
        "BeatEngine Game",
        sf::Style::Default,
        (gameSettings->WindowFullScreen ? sf::State::Fullscreen : sf::State::Windowed)
    );
    if (gameSettings->WindowFullScreen)
        m_Flags |= GameFlags_Fullscreen;
    else 
        m_Flags &= ~GameFlags_Fullscreen;

	this->m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(gameSettings->WindowSize.x), static_cast<float>(gameSettings->WindowSize.y) }));
	this->m_Window->setFramerateLimit(gameSettings->FpsLimit);
    this->m_Window->setVerticalSyncEnabled(gameSettings->VSync);
	this->m_Window->setView(m_View);
    this->m_Window->setMouseCursor(m_Cursor);

	if (!ImGui::SFML::Init(*m_Window)) {
        m_Window->close();
    }
}

void Game::InitKeybinds() {
	Logger::AddInfo(typeid(Game), "Initializing keybinds... (not really)");
}

void Game::SubscribeToGameEvent() {
	Logger::AddInfo(typeid(Game), "Subscribing to game events...");

    EventManager::GetInstance()->Subscribe<GameSettingsChanged>([this](std::shared_ptr<Base::Event> event) {
        
        auto settings = std::static_pointer_cast<GameSettings>(m_SettingsMgr->GetSettings(typeid(GameSettings)));
        
        auto curFullscreen = m_Flags & GameFlags_Fullscreen;

        if (settings->WindowFullScreen != curFullscreen) {
            ImGui::SFML::Shutdown();
            m_Window->close();
        	this->m_Window = new sf::RenderWindow(
	            sf::VideoMode(settings->WindowSize), 
                "BeatEngine Game",
                sf::Style::Default,
                (settings->WindowFullScreen ? sf::State::Fullscreen : sf::State::Windowed)
            );
            if (settings->WindowFullScreen)
                m_Flags |= GameFlags_Fullscreen;
            else
                m_Flags &= ~GameFlags_Fullscreen;

            if (!ImGui::SFML::Init(*m_Window)) {
                m_Window->close();
            }
        }

        m_Window->setFramerateLimit(settings->FpsLimit);
        m_Window->setSize(settings->WindowSize);
        m_Window->setVerticalSyncEnabled(settings->VSync);
        m_Window->setMouseCursor(m_Cursor);

    });

}

void Game::SubscribeToGameSignals() {
	Logger::AddInfo(typeid(Game), "Subscribing to game signals...");

	SignalManager::GetInstance()->RegisterCallback<ViewAddGlobalLayerSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
		auto signal = std::static_pointer_cast<ViewAddGlobalLayerSignal>(sig);
		this->m_GlobalLayers.AttachLayer(signal->Layer);

		signal->Layer = nullptr;
	});

    SignalManager::GetInstance()->RegisterCallback<GameExitSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        EventManager::GetInstance()->Send(std::make_shared<GameExitingEvent>());
        m_Cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
        m_Window->setMouseCursor(m_Cursor);
        m_Running = false;
        // exit(0);
    });

    SignalManager::GetInstance()->RegisterCallback<GameChangeCursorSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameChangeCursorSignal>(sig);
        m_Cursor = sf::Cursor::createFromSystem(gameSig->NewCursor).value();
        m_Flags |= GameFlags_CursorChanged;
    });

    SignalManager::GetInstance()->RegisterCallback<GameToggleImGui>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameToggleImGui>(sig);
        if (m_Flags & GameFlags_ImGui)
            m_Flags &= ~GameFlags_ImGui;
        else
            m_Flags |= GameFlags_ImGui;
    });

    SignalManager::GetInstance()->RegisterCallback<GameAddFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameAddFlags>(sig);
        this->SetFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<GameRemoveFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameRemoveFlags>(sig);
        this->RemoveFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<GameToggleDrawingDebugInfo>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
            if (m_Flags & GameFlags_DrawDebugInfo)
                m_Flags &= ~GameFlags_DrawDebugInfo;
            else
                m_Flags |= GameFlags_DrawDebugInfo;
    });
}
