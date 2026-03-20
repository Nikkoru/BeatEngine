#include "BeatEngine/Game.h"

#include <cstdlib>
// #include <imgui.h>
// #include <imgui_internal.h>
// #include <imgui-SFML.h>
#include <memory>

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/AudioStream.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/ViewManager.h"
#include "BeatEngine/Settings/GameSettings.h"

#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/System/Time.h"

Game::Game() {
    m_Context = std::make_shared<GameContext>();
    m_State = std::make_shared<GameState>();
    m_State->ViewMgr = std::make_shared<ViewManager>(m_Context, m_State);
    m_State->GraphicsMgr = std::make_shared<GraphicsManager>(m_Context, m_State);
    m_State->UIMgr = std::make_shared<UIManager>(m_Context, m_State);
    m_State->SystemMgr = std::make_shared<SystemManager>(m_Context, m_State);
    m_State->SettingsMgr = std::make_shared<SettingsManager>(m_Context, m_State);
    m_State->AssetMgr = std::make_shared<AssetManager>(m_Context, m_State);

    // m_AudioMgr = new AudioManager{ m_Context };
#ifdef BEATENGINE_DEBUG
    Logger::AddInfo("", "Debug stuff");
    m_Context->EFlags |= EnvFlags_Debug;
#endif
#ifdef BEATENGINE_TEST
    Logger::AddInfo("", "This is a Test Build");
    m_Context->EFlags |= EnvFlags_TestBuild;
#endif
}

Game::~Game() {
	// ImGui::SFML::Shutdown();
}

void Game::Run() {
    m_Running = true;
	Logger::AddInfo(typeid(Game), "Game started!");

	if (!m_State->ViewMgr->HasActiveViews())
		m_State->ViewMgr->Push(m_State->ViewMgr->MainView);


	if (m_Context->GFlags & GameFlags_Preload) {
		m_State->SettingsMgr->ReadConfig(m_SettingsPath);
		ApplyBaseSettings();
	}

	while (m_State->GraphicsMgr->IsOpen() && m_Running) {
		while (const auto event = m_State->GraphicsMgr->PollEvent()) {
			// if (m_Context->GFlags & GameFlags_ImGui)
			// 	ImGui::SFML::ProcessEvent(*m_Window, *event);
   //          if ((event->Is<sf::Event::KeyPressed>() || event->Is<sf::Event::KeyReleased>()) && (m_Context->GFlags & GameFlags_DisableKeyPressEvents))
   //              continue;
			// if (event->Is<sf::Event::Closed>()) {
   //              EventManager::GetInstance()->Send(std::make_shared<GameExitingEvent>());
   //              m_Running = false;
   //          }
			// if (auto data = event->GetIf<sf::Event::Resized>()) {
			// 	// m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(data->size.x), static_cast<float>(data->size.y) }));
			// 	// m_Window->setView(m_View);
			//
   //              EventManager::GetInstance()->Send(std::make_shared<GameResized>(data->size));
			// }
            if (m_State->GraphicsMgr->IsOpen() && m_Running) {
		    	m_GlobalLayers.OnEvent(event);
            }
            else
                break;

            if (m_State->GraphicsMgr->IsOpen() && m_Running) {
			    if (!m_State->ViewMgr->OnEvent(event)) {
                    m_State->GraphicsMgr->Close();
				    break;
			    }
            }
            else {
                m_State->GraphicsMgr->Close();
                break;
            }
		}
        if (m_State->GraphicsMgr->IsOpen() && m_Running) {
		    this->Update();
		    this->Draw();
		    this->Display();
        }
        else
            break;
	}
}

void Game::Initialize() {
    Logger::AddInfo(typeid(Game), "Initializing Game");

    InitSettings();
	InitAudio();
	InitAssets();
	InitSystems();
	InitWindow();
	InitUI();
	InitViews();
	InitKeybinds();

	SubscribeToGameEvent();
	SubscribeToGameSignals();
}

void Game::Uninitialize() {
    Logger::AddInfo(typeid(Game), "Game in shutdown");

    // m_State->KeybindsMgr->Uninit();
    // m_State->ViewMgr->Uninit();
    // m_State->UIMgr->Uninit();
    m_State->GraphicsMgr->Close();
    m_State->SystemMgr->StopSystems();
    // m_AssetMgr->Uninit();
    // m_AudioMgr->Uninit();
    // m_SettingsMgr->Uninit();
}

void Game::UseImGui(bool show) {
    if (show)
        m_Context->GFlags |= GameFlags_ImGui;
    else
        m_Context->GFlags &= ~GameFlags_ImGui;
}

void Game::UseImGuiDocking(bool docking) {
    // if (docking)
    //     ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // else
    //     ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
}

void Game::SetWindowSize(Vector2u size) {
    auto settings = std::static_pointer_cast<GameSettings>(m_State->SettingsMgr->GetSettings(typeid(GameSettings)));
    settings->WindowSize = size;

	m_State->GraphicsMgr->GetWindow()->SetSize(size);
}

void Game::SetWindowTitle(std::string title) {
    m_State->GraphicsMgr->SetWindowTitle(title);
}

void Game::PreloadSettings() {
    m_Context->GFlags |= GameFlags_Preload;

	m_State->SettingsMgr->ReadConfig(m_SettingsPath);
}

void Game::SaveSettings() {
	m_State->SettingsMgr->WriteConfig(m_SettingsPath);
}

void Game::SetConfigPath(std::filesystem::path path) {
	this->m_SettingsPath = path;
}

void Game::SetFlags(GameFlags flags) {
    this->m_Context->GFlags |= flags;
}

void Game::RemoveFlags(GameFlags flags) {
    this->m_Context->GFlags &= ~flags;
}

void Game::DrawImGuiDebug() {
    // ImGui::Begin("BeatEngine Game Debug Window");
    bool imguiToggle = m_Context->GFlags & GameFlags_ImGui;
    bool preloadToggle = m_Context->GFlags & GameFlags_Preload;
    bool fullscreenToggle = m_Context->GFlags & GameFlags_Fullscreen;
    bool cursorChangedToggle = m_Context->GFlags & GameFlags_CursorChanged;
    bool disableKeysToggle = m_Context->GFlags & GameFlags_DisableKeyPressEvents;
    bool drawDebugToggle = m_Context->GFlags & GameFlags_DrawDebugInfo;

    bool viewDisableKeyToggle = m_Context->VFlags & ViewFlags_DisableKeys;


    // ImGui::Checkbox("GameFlags_ImGui", &imguiToggle);
    // ImGui::Checkbox("GameFlags_Preload", &preloadToggle);
    // ImGui::Checkbox("GameFlags_Fullscreen", &fullscreenToggle);
    // ImGui::Checkbox("GameFlags_CursorChanged", &cursorChangedToggle);
    // ImGui::Checkbox("GameFlags_DisableKeyPressEvents", &disableKeysToggle);
    // ImGui::Checkbox("GameFlags_DrawDebugInfo", &drawDebugToggle);
    // ImGui::NewLine();
    // ImGui::Checkbox("ViewFlags_DisableKeys", &viewDisableKeyToggle);
    // ImGui::End();
}

void Game::LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets) {
	if (globalAssets.empty())
		return;
	for (auto& [type, vecPath] : globalAssets) {
		switch (type) {
		case AssetType::Texture:
			for (auto& path : vecPath)
				m_State->AssetMgr->Load<Texture>(path);
			break;
		case AssetType::AudioStream:
			for (auto& path : vecPath)
				m_State->AssetMgr->Load<AudioStream>(path);
			break;
		case AssetType::Sound:
			for (auto& path : vecPath)
				m_State->AssetMgr->Load<Sound>(path);
			break;
		case AssetType::Font:
			for (auto& path : vecPath)
				m_State->AssetMgr->Load<Font>(path);
        case AssetType::Shader:
            for (auto& path : vecPath)
                m_State->AssetMgr->Load<Shader>(path);
		}
            
	}
}

void Game::Display() {
// if (m_Context->GFlags & GameFlags_ImGui)
// 		ImGui::SFML::Render(*m_Window);
//
	m_State->GraphicsMgr->Display();
}

void Game::Draw() {
    m_State->GraphicsMgr->Render();
    m_State->GraphicsMgr->Clear();
//
// 	// m_AnimationMgr->DrawActiveAnimation(m_DeltaClock);
// 	if (!m_ViewMgr->OnDraw(m_Window))
// 		m_Window->close();
//
// 	m_Window->draw(m_GlobalLayers);
//
//     m_UIMgr->OnDraw(m_Window);
//
//     if ((m_Context->GFlags & GameFlags_DrawDebugInfo) && (m_Context->GFlags & GameFlags_ImGui))
//         DrawImGuiDebug();
}

void Game::Update() {
    // m_Context->WindowSize = m_Window->getSize();
    //
    // if (m_Context->GFlags & GameFlags_CursorChanged) {
    //     m_Window->setMouseCursor(m_Cursor);
    //     m_Context->GFlags &= ~GameFlags_CursorChanged;
    // }

	auto sfDelta = m_Clock.GetAndReset();
	auto deltaTime = sfDelta.AsSeconds();

	if (m_Context->GFlags & GameFlags_ImGui)
		// ImGui::SFML::Update(*m_Window, sfDelta);

	if (!this->m_State->ViewMgr->OnUpdate(deltaTime)) {
		// this->m_Window->close();
		return;
	}

    m_State->GraphicsMgr->Update();
	
    this->m_State->SystemMgr->Update(deltaTime);
	m_GlobalLayers.OnUpdate(deltaTime);
    m_State->UIMgr->Update(deltaTime);
}

void Game::ApplyBaseSettings() {
	auto gameSettings = std::static_pointer_cast<GameSettings>(m_State->SettingsMgr->GetSettings(typeid(GameSettings)));

	// m_Window->setFramerateLimit(gameSettings->FpsLimit);
	// m_Window->setSize(gameSettings->WindowSize);
	//
	// if (gameSettings->WindowPosition != sf::Vector2i(-1, -1))
	// 	m_Window->setPosition(gameSettings->WindowPosition);
}

void Game::InitSettings() {
	Logger::AddInfo(typeid(Game), "Initializing settings...");
    
	m_State->SettingsMgr->RegisterSettingsData<GameSettings>();
    m_State->SettingsMgr->ReadConfig(m_SettingsPath);
}

void Game::InitUI() {
	Logger::AddInfo(typeid(Game), "Initializing UI...");
}

void Game::InitAudio() {
	Logger::AddInfo(typeid(Game), "Initializing audio...");
    Logger::AddWarning(typeid(Game), "AudioManager is in a broken state. It is not recomended to use it. Any call for this manager please comment it out as AudioManager is not initialized. You can still use it if BEATENGINE_TEST is defined.");
#ifdef BEATENGINE_TEST
	this->m_AudioMgr = new AudioManager(m_Context);
#endif
}

void Game::InitViews() {
	Logger::AddInfo(typeid(Game), "Initializing views...");
}

void Game::InitSystems() {
    Logger::AddInfo(typeid(Game), "Initializing systems...");
}

void Game::InitAssets() {
	Logger::AddInfo(typeid(Game), "Initializing assets...");
}

void Game::InitWindow() {
	Logger::AddInfo(typeid(Game), "Initializing window...");

    m_State->GraphicsMgr->Init();

	auto settings = m_State->SettingsMgr->GetSettings(typeid(GameSettings));
	auto gameSettings = std::static_pointer_cast<GameSettings>(settings);
    
    // if (gameSettings->WindowFullScreen)
	   //  this->m_Window = new sf::RenderWindow(
	   //      sf::VideoMode{}, 
    //         "BeatEngine Game",
    //         sf::Style::Default,
    //         sf::State::Fullscreen 
    //     );
    // else {
	   //  this->m_Window = new sf::RenderWindow(
	   //      sf::VideoMode(gameSettings->WindowSize), 
    //         "BeatEngine Game",
    //         sf::Style::Default,
    //         sf::State::Windowed
    //     );
    // }
    //
    if (gameSettings->WindowFullScreen)
        m_Context->GFlags |= GameFlags_Fullscreen;

	// this->m_View = sf::View(sf::FloatRect({ 0, 0 }, { static_cast<float>(gameSettings->WindowSize.x), static_cast<float>(gameSettings->WindowSize.y) }));
	// this->m_Window->setFramerateLimit(gameSettings->FpsLimit);
 //    this->m_Window->setVerticalSyncEnabled(gameSettings->VSync);
	// this->m_Window->setView(m_View);
 //    this->m_Window->setMouseCursor(m_Cursor);
	//
 //    this->m_Context->WindowSize = m_Window->getSize();

// 	if (!ImGui::SFML::Init(*m_Window)) {
//         m_Window->close();
//     }
}

void Game::InitKeybinds() {
	Logger::AddInfo(typeid(Game), "Initializing keybinds... (not really)");
}

void Game::SubscribeToGameEvent() {
	Logger::AddInfo(typeid(Game), "Subscribing to game events...");

    EventManager::GetInstance()->Subscribe<EventGameSettingsChanged>([this](std::shared_ptr<Base::Event> event) {
        
        auto settings = std::static_pointer_cast<GameSettings>(m_State->SettingsMgr->GetSettings(typeid(GameSettings)));
        
        bool curFullscreen = m_Context->GFlags & GameFlags_Fullscreen;

        if (settings->WindowFullScreen != curFullscreen) {
            // m_Window->close();
            // delete m_Window;
            // if (settings->WindowFullScreen)
            //     this->m_Window = new sf::RenderWindow(
            //         sf::VideoMode{}, 
            //         "BeatEngine Game",
            //         sf::Style::Default,
            //         sf::State::Fullscreen 
            //     );
            // else {
            //     this->m_Window = new sf::RenderWindow(
            //         sf::VideoMode(settings->WindowSize), 
            //         "BeatEngine Game",
            //         sf::Style::Default,
            //         sf::State::Windowed
            //     );
            // }

            if (settings->WindowFullScreen)
                m_Context->GFlags |= GameFlags_Fullscreen;
            else if (m_Context->GFlags & GameFlags_Fullscreen)
                m_Context->GFlags &= ~GameFlags_Fullscreen;
        //     m_Window->display();
        }

        // m_Window->setFramerateLimit(settings->FpsLimit);
        // if (!settings->WindowFullScreen && m_Window->getSize() != settings->WindowSize) {
        //     m_Window->setSize(settings->WindowSize);
        // }
        // m_Window->setVerticalSyncEnabled(settings->VSync);
        // m_Window->setMouseCursor(m_Cursor);
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
        EventManager::GetInstance()->Send(std::make_shared<EventGameExiting>());
        // m_Cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
        // m_Window->setMouseCursor(m_Cursor);
        Uninitialize();
        m_Running = false;
        // exit(0);
    });

    // SignalManager::GetInstance()->RegisterCallback<GameChangeCursorSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
    //     auto gameSig = std::static_pointer_cast<GameChangeCursorSignal>(sig);
    //     // m_Cursor = sf::Cursor::createFromSystem(gameSig->NewCursor).value();
    //     m_Context->GFlags |= GameFlags_CursorChanged;
    // });

    SignalManager::GetInstance()->RegisterCallback<GameToggleImGui>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameToggleImGui>(sig);
        if (m_Context->GFlags & GameFlags_ImGui)
            m_Context->GFlags &= ~GameFlags_ImGui;
        else
            m_Context->GFlags |= GameFlags_ImGui;
    });

    SignalManager::GetInstance()->RegisterCallback<GameAddFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameAddFlags>(sig);
        this->SetFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<GameRemoveFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<GameRemoveFlags>(sig);
        this->RemoveFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<ViewAddFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<ViewAddFlags>(sig);
        this->m_Context->VFlags |= gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<ViewRemoveFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<ViewAddFlags>(sig);
        this->m_Context->VFlags &= ~gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<GameToggleDrawingDebugInfo>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
            if (m_Context->GFlags & GameFlags_DrawDebugInfo)
                m_Context->GFlags &= ~GameFlags_DrawDebugInfo;
            else
                m_Context->GFlags |= GameFlags_DrawDebugInfo;
    });
}
