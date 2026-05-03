#include "BeatEngine/Game.h"

#include <cstddef>
#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>
#include <memory>

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/AssetType.h"
#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/ViewManager.h"

#include "BeatEngine/Settings/GameSettings.h"
#include "BeatEngine/Settings/GameDebugSettings.h"

#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/System/Time.h"

Game::Game() {
    m_State.CreateManagers(&m_Context);
#ifdef BEATENGINE_DEBUG
    // Logger::PrintDebug(true);
    Logger::AddInfo("", "Debug Build");
    m_Context.EFlags |= EnvFlags_Debug;
#endif
#ifdef BEATENGINE_TEST
    Logger::AddInfo("", "This is a Test Build");
    m_Context->EFlags |= EnvFlags_TestBuild;
#endif
}

Game::~Game() {
}

void Game::Run() {
    m_Running = true;
	Logger::AddInfo(typeid(Game), "Game started!");

	if (!m_State.GetViewMgr().HasActiveViews())
		m_State.GetViewMgr().Push(m_State.GetViewMgr().MainView);


	if (m_Context.GFlags & GameFlags_Preload) {
		m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);
		ApplyBaseSettings();
	}

	while (m_State.GetGraphicsMgr().IsOpen() && m_Running) {
		while (auto event = m_State.GetGraphicsMgr().PollEvent()) {
            if (event->Is<GameExitingEvent>())
                Uninitialize();
			// if (m_Context->GFlags & GameFlags_ImGui)
			// 	ImGui::SFML::ProcessEvent(*m_Window, *event);
            if (m_State.GetGraphicsMgr().IsOpen() && m_Running) {
		    	m_GlobalLayers.OnEvent(event);
            }
            else
                break;
            if (m_State.GetGraphicsMgr().IsOpen() && m_Running) {
			    if (!m_State.GetViewMgr().OnEvent(event)) {
                    m_State.GetGraphicsMgr().Close();
				    break;
			    }
            }
            else {
                m_State.GetGraphicsMgr().Close();
                break;
            }
		}
        if (m_State.GetGraphicsMgr().IsOpen() && m_Running) {
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
	InitSystems();
	InitWindow();
	InitAssets();
	InitUI();
	InitViews();
	InitKeybinds();

	SubscribeToGameEvent();
	SubscribeToGameSignals();
}

void Game::Uninitialize() {
    Logger::AddInfo(typeid(Game), "Game in shutdown");

    // m_State->KeybindsMgr->Uninit();
    m_State.GetViewMgr().Uninit();
    // m_State->UIMgr->Uninit();
    m_State.GetGraphicsMgr().Close();
    m_State.GetSystemMgr().StopSystems();
    // m_AssetMgr->Uninit();
    m_State.GetAudioMgr().Uninit();
    // m_SettingsMgr->Uninit();
    m_Running = false;
}

void Game::UseImGui(bool show) {
    if (show)
        m_Context.GFlags |= GameFlags_ImGui;
    else
        m_Context.GFlags &= ~GameFlags_ImGui;
}

void Game::UseImGuiDocking(bool docking) {
    if (docking)
        m_Context.GFlags |= GameFlags_ImGuiDocking;
    else
        m_Context.GFlags &= ~GameFlags_ImGuiDocking;
}

void Game::SetWindowSize(Vector2u size) {
    if (m_State.GetSettingsMgr().HasSettings<GameSettings>()) {
        auto settings = std::static_pointer_cast<GameSettings>(m_State.GetSettingsMgr().GetSettings(typeid(GameSettings)));
        settings->WindowSize = size;
    }
    m_State.GetGraphicsMgr().SetWindowSize(size);
}

void Game::SetWindowTitle(std::string title) {
    m_State.GetGraphicsMgr().SetWindowTitle(title);
}

void Game::PreloadSettings() {
    m_Context.GFlags |= GameFlags_Preload;

	m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);
}

void Game::SaveSettings() {
	m_State.GetSettingsMgr().WriteConfig(m_SettingsPath);
}

void Game::SetConfigPath(std::filesystem::path path) {
	this->m_SettingsPath = path;
}

void Game::SetFlags(GameFlags flags) {
    this->m_Context.GFlags |= flags;
}

void Game::RemoveFlags(GameFlags flags) {
    this->m_Context.GFlags &= ~flags;
}

void Game::DrawImGuiDebug() {
    static bool editFlags = false;


    ImGui::Begin("BeatEngine Game Debug Window", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Environment")) {
            bool dockingStatus = (m_Context.GFlags & GameFlags_DebugDock);
            if (ImGui::MenuItem("Enable Docking", NULL, dockingStatus)) {
                !dockingStatus ? m_Context.GFlags |= GameFlags_DebugDock :
                            m_Context.GFlags &= ~GameFlags_DebugDock;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("GameActionBar")) {
        if (ImGui::BeginTabItem("Flags")) {
            bool imguiToggle = m_Context.GFlags & GameFlags_ImGui;
            bool imguiDockingToggle = m_Context.GFlags & GameFlags_ImGuiDocking;
            bool runningToggle = m_Context.GFlags & GameFlags_Running;
            bool preloadToggle = m_Context.GFlags & GameFlags_Preload;
            bool fullscreenToggle = m_Context.GFlags & GameFlags_Fullscreen;
            bool cursorChangedToggle = m_Context.GFlags & GameFlags_CursorChanged;
            bool disableKeysToggle = m_Context.GFlags & GameFlags_DisableKeyPressEvents;
            bool drawDebugToggle = m_Context.GFlags & GameFlags_DrawDebugInfo;
            bool drawDockToggle = m_Context.GFlags & GameFlags_DebugDock;

            bool viewDisableKeyToggle = m_Context.VFlags & ViewFlags_DisableKeys;

            bool envDebugToggle = m_Context.EFlags & EnvFlags_Debug;
            bool envTestToggle = m_Context.EFlags & EnvFlags_TestBuild;
            
            ImGui::Text("GameFlags: %#.8x", m_Context.GFlags);
            if (!editFlags)
                ImGui::BeginDisabled();
            ImGui::Checkbox("GameFlags_ImGui", &imguiToggle);
            ImGui::Checkbox("GameFlags_ImGuiDocking", &imguiDockingToggle);
            ImGui::Checkbox("GameFlags_Running", &runningToggle);
            ImGui::Checkbox("GameFlags_Preload", &preloadToggle);
            ImGui::Checkbox("GameFlags_Fullscreen", &fullscreenToggle);
            ImGui::Checkbox("GameFlags_CursorChanged", &cursorChangedToggle);
            ImGui::Checkbox("GameFlags_DisableKeyPressEvents", &disableKeysToggle);
            ImGui::Checkbox("GameFlags_DrawDebugInfo", &drawDebugToggle);
            ImGui::Checkbox("GameFlags_DebugDock", &drawDockToggle);
            if (!editFlags)
                ImGui::EndDisabled();
            ImGui::NewLine();
            ImGui::Text("ViewFlags: %#.8x", m_Context.VFlags);
            if (!editFlags)
                ImGui::BeginDisabled();
            ImGui::Checkbox("ViewFlags_DisableKeys", &viewDisableKeyToggle);
            if (!editFlags)
                ImGui::EndDisabled();
            ImGui::NewLine();
            ImGui::Text("EnvFlags: %#.8x", m_Context.EFlags);
            if (!editFlags)
                ImGui::BeginDisabled();
            ImGui::Checkbox("EnvFlags_Debug", &envDebugToggle);
            ImGui::Checkbox("EnvFlags_TestBuild", &envTestToggle);
            if (!editFlags)
                ImGui::EndDisabled();

            if (!imguiToggle && m_Context.GFlags & GameFlags_ImGui)
                m_Context.GFlags &= GameFlags_ImGui;
            else if (imguiToggle && !(m_Context.GFlags &GameFlags_ImGui))
                m_Context.GFlags |= ~GameFlags_ImGui;

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Actions")) {
            if (ImGui::Button("Exit"))
                SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
            ImGui::Checkbox("Allow editing flags", &editFlags);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Status")) {
            ImGui::Text("Delta: %.3f (%.1f ms)", LastDelta, LastDelta * 1000);
            ImGui::Text("FPS: %.2f", 1 / LastDelta);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Context")) {
            auto size = m_Context.WindowSize;
            ImGui::Text("WindowSize: (X: %u Y: %u)", size.X, size.Y);
            ImGui::Text("ActiveView: %s", m_Context.ActiveView.name());

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Game::LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets) {
	if (globalAssets.empty())
		return;
    size_t assets{};
	for (const auto& [type, vecPath] : globalAssets) {
        auto vecSize = vecPath.size();
        for (const auto& path : vecPath) {
            if (!m_State.GetAssetMgr().Preload(type, path))
            vecSize--;
        }
        assets += vecSize;
	}

    Logger::AddDebug(typeid(Game), "Preloaded {} assets", assets);
}

void Game::Display() {
    if (m_Context.GFlags & GameFlags_ImGui && m_Context.GFlags & GameFlags_DrawDebugInfo) {
        DrawImGuiDebug();
    }

    m_State.GetGraphicsMgr().Clear();
	m_State.GetGraphicsMgr().Display();
}

void Game::Draw() {
    m_State.GetGraphicsMgr().Render();

    if (m_Context.GFlags & GameFlags_DebugDock && 
        m_Context.GFlags & GameFlags_ImGui &&
        m_Context.GFlags & GameFlags_ImGuiDocking &&
        m_Context.GFlags & GameFlags_DrawDebugInfo) {
        auto io = ImGui::GetIO();
        auto viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

        
        ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiWindowFlags_NoDocking;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus | 
            ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(.0f, .0f));
        ImGui::Begin("##DockWindow", nullptr, windowFlags);
        ImGui::PopStyleVar();
        auto dockspaceId = ImGui::GetID("DebugDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockFlags);
        ImGui::End();
    }

	if (!m_State.GetViewMgr().OnDraw())
		Uninitialize();

	m_GlobalLayers.OnDraw();
    m_State.GetUIMgr().OnDraw();
}

void Game::Update() {
    m_Context.WindowSize = m_State.GetGraphicsMgr().GetWindow()->GetSize();
    //
    // if (m_Context->GFlags & GameFlags_CursorChanged) {
    //     m_Window->setMouseCursor(m_Cursor);
    //     m_Context->GFlags &= ~GameFlags_CursorChanged;
    // }

	auto sfDelta = m_Clock.GetAndReset();
	auto deltaTime = sfDelta.AsSeconds();

	if (!this->m_State.GetViewMgr().OnUpdate(deltaTime)) {
        Uninitialize();
		return;
	}

    m_State.GetGraphicsMgr().Update();
	
    this->m_State.GetSystemMgr().Update(deltaTime);
	m_GlobalLayers.OnUpdate(deltaTime);
    m_State.GetUIMgr().Update(deltaTime);

    LastDelta = deltaTime;
}

void Game::ApplyBaseSettings() {
	auto gameSettings = std::static_pointer_cast<GameSettings>(m_State.GetSettingsMgr().GetSettings(typeid(GameSettings)));
    auto window = m_State.GetGraphicsMgr().GetWindow();

    m_State.GetGraphicsMgr().SetFramerateLimit(gameSettings->FpsLimit);

	window->SetSize(gameSettings->WindowSize);
	if (gameSettings->WindowPosition != Vector2i(-1, -1))
		window->SetPosition(gameSettings->WindowPosition);
}

void Game::InitSettings() {
	Logger::AddDebug(typeid(Game), "Initializing settings...");
    
	m_State.GetSettingsMgr().RegisterSettingsData<GameSettings>();
    m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);

#ifdef BEATENGINE_DEBUG
    m_State.GetSettingsMgr().RegisterSettingsData<GameDebugSettings>();
    m_State.GetSettingsMgr().ReadConfig("debug.ini");
#endif
}

void Game::InitUI() {
	Logger::AddDebug(typeid(Game), "Initializing UI...");
}

void Game::InitAudio() {
	Logger::AddDebug(typeid(Game), "Initializing audio...");
    Logger::AddWarning(typeid(Game), "AudioManager is in a broken state. It is not recomended to use it. Any call for this manager please comment it out as AudioManager is not initialized. You can still use it if BEATENGINE_TEST is defined.");

    m_State.GetAudioMgr().Init();
}

void Game::InitViews() {
	Logger::AddDebug(typeid(Game), "Initializing views...");

    m_State.GetViewMgr().Init();
}

void Game::InitSystems() {
    Logger::AddDebug(typeid(Game), "Initializing systems...");
}

void Game::InitAssets() {
	Logger::AddDebug(typeid(Game), "Initializing assets...");

    m_State.GetAssetMgr().Init();
}

void Game::InitWindow() {
	Logger::AddDebug(typeid(Game), "Initializing window...");

	auto settings = m_State.GetSettingsMgr().GetSettings(typeid(GameSettings));
	auto gameSettings = std::static_pointer_cast<GameSettings>(settings);

    m_State.GetGraphicsMgr().SetWindowFullscreen(gameSettings->WindowFullScreen);
    m_State.GetGraphicsMgr().Init();

    if (gameSettings->WindowFullScreen) {
        m_Context.GFlags |= GameFlags_Fullscreen;
    }

    m_Context.WindowSize = m_State.GetGraphicsMgr().GetWindow()->GetSize();
}

void Game::InitKeybinds() {
	Logger::AddDebug(typeid(Game), "Initializing keybinds... (not really)");
}

void Game::SubscribeToGameEvent() {
	Logger::AddDebug(typeid(Game), "Subscribing to game events...");

    EventManager::GetInstance()->Subscribe<GameSettingsChangedEvent>([this](std::shared_ptr<Base::Event> event) {
        
        auto settings = std::static_pointer_cast<GameSettings>(m_State.GetSettingsMgr().GetSettings(typeid(GameSettings)));
        
        bool curFullscreen = m_Context.GFlags & GameFlags_Fullscreen;

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
                m_Context.GFlags |= GameFlags_Fullscreen;
            else if (m_Context.GFlags & GameFlags_Fullscreen)
                m_Context.GFlags &= ~GameFlags_Fullscreen;
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
	Logger::AddDebug(typeid(Game), "Subscribing to game signals...");

	SignalManager::GetInstance()->RegisterCallback<ViewAddGlobalLayerSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
		auto signal = std::static_pointer_cast<ViewAddGlobalLayerSignal>(sig);
		this->m_GlobalLayers.AttachLayer(signal->Layer);

		signal->Layer = nullptr;
	});

    SignalManager::GetInstance()->RegisterCallback<GameExitSignal>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        EventManager::GetInstance()->Send(std::make_shared<GameExitingEvent>());
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
        if (m_Context.GFlags & GameFlags_ImGui)
            m_Context.GFlags &= ~GameFlags_ImGui;
        else
            m_Context.GFlags |= GameFlags_ImGui;
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
        this->m_Context.VFlags |= gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<ViewRemoveFlags>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<ViewAddFlags>(sig);
        this->m_Context.VFlags &= ~gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<GameToggleDrawingDebugInfo>(typeid(Game), [this](const std::shared_ptr<Base::Signal> sig) {
            if (m_Context.GFlags & GameFlags_DrawDebugInfo)
                m_Context.GFlags &= ~GameFlags_DrawDebugInfo;
            else
                m_Context.GFlags |= GameFlags_DrawDebugInfo;
    });
}
