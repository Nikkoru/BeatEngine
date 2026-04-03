#include "BeatEngine/Game.h"

#include <cstddef>
#include <cstdlib>
#include <imgui.h>
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
		while (const auto event = m_State.GetGraphicsMgr().PollEvent()) {
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
    m_State.GetGraphicsMgr().Close();
    m_State.GetSystemMgr().StopSystems();
    // m_AssetMgr->Uninit();
    // m_AudioMgr->Uninit();
    // m_SettingsMgr->Uninit();
}

void Game::UseImGui(bool show) {
    if (show)
        m_Context.GFlags |= GameFlags_ImGui;
    else
        m_Context.GFlags &= ~GameFlags_ImGui;
}

void Game::UseImGuiDocking(bool docking) {
    // if (docking)
    //     ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // else
    //     ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
}

void Game::SetWindowSize(Vector2u size) {
    auto settings = std::static_pointer_cast<GameSettings>(m_State.GetSettingsMgr().GetSettings(typeid(GameSettings)));
    settings->WindowSize = size;

	m_State.GetGraphicsMgr().GetWindow()->SetSize(size);
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
    ImGui::Begin("BeatEngine Game Debug Window");
    bool imguiToggle = m_Context.GFlags & GameFlags_ImGui;
    bool preloadToggle = m_Context.GFlags & GameFlags_Preload;
    bool fullscreenToggle = m_Context.GFlags & GameFlags_Fullscreen;
    bool cursorChangedToggle = m_Context.GFlags & GameFlags_CursorChanged;
    bool disableKeysToggle = m_Context.GFlags & GameFlags_DisableKeyPressEvents;
    bool drawDebugToggle = m_Context.GFlags & GameFlags_DrawDebugInfo;

    bool viewDisableKeyToggle = m_Context.VFlags & ViewFlags_DisableKeys;


    ImGui::Checkbox("GameFlags_ImGui", &imguiToggle);
    ImGui::Checkbox("GameFlags_Preload", &preloadToggle);
    ImGui::Checkbox("GameFlags_Fullscreen", &fullscreenToggle);
    ImGui::Checkbox("GameFlags_CursorChanged", &cursorChangedToggle);
    ImGui::Checkbox("GameFlags_DisableKeyPressEvents", &disableKeysToggle);
    ImGui::Checkbox("GameFlags_DrawDebugInfo", &drawDebugToggle);
    ImGui::NewLine();
    ImGui::Checkbox("ViewFlags_DisableKeys", &viewDisableKeyToggle);
    ImGui::End();
}

void Game::LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets) {
	if (globalAssets.empty())
		return;
    size_t assets{};
	for (auto& [type, vecPath] : globalAssets) {
        auto vecSize = vecPath.size();
		switch (type) {
		case AssetType::Texture:
			for (auto& path : vecPath)
				if (!m_State.GetAssetMgr().Load<Texture>(path))
                    vecSize--;
			break;
		case AssetType::AudioStream:
			for (auto& path : vecPath)
				if (!m_State.GetAssetMgr().Load<AudioStream>(path))
                    vecSize--;
			break;
		case AssetType::Sound:
			for (auto& path : vecPath)
				if (!m_State.GetAssetMgr().Load<Sound>(path))
                    vecSize--;
			break;
		case AssetType::Font:
			for (auto& path : vecPath)
				if (!m_State.GetAssetMgr().Load<Font>(path))
                    vecSize--;
        case AssetType::VertexShader:
            for (auto& path : vecPath)
                if (!m_State.GetAssetMgr().LoadShader(path, Shader::Type::Vertex))
                    vecSize--;
            break;
        case AssetType::FragmentShader:
            for (auto& path : vecPath)
                if (!m_State.GetAssetMgr().LoadShader(path, Shader::Type::Fragment))
                    vecSize--;
		}
        assets += vecSize;
	}

    Logger::AddDebug(typeid(Game), "Created {} assets", assets);
}

void Game::Display() {
    if (m_Context.GFlags & GameFlags_ImGui)
        m_State.GetGraphicsMgr().RenderImGui();

	m_State.GetGraphicsMgr().Display();
}

void Game::Draw() {
    m_State.GetGraphicsMgr().Render();
    m_State.GetGraphicsMgr().Clear();
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

	if (m_Context.GFlags & GameFlags_ImGui)
		// ImGui::SFML::Update(*m_Window, sfDelta);

	if (!this->m_State.GetViewMgr().OnUpdate(deltaTime)) {
		// this->m_Window->close();
		return;
	}

    m_State.GetGraphicsMgr().Update();
	
    this->m_State.GetSystemMgr().Update(deltaTime);
	m_GlobalLayers.OnUpdate(deltaTime);
    m_State.GetUIMgr().Update(deltaTime);
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
}

void Game::InitUI() {
	Logger::AddDebug(typeid(Game), "Initializing UI...");
}

void Game::InitAudio() {
	Logger::AddDebug(typeid(Game), "Initializing audio...");
    Logger::AddWarning(typeid(Game), "AudioManager is in a broken state. It is not recomended to use it. Any call for this manager please comment it out as AudioManager is not initialized. You can still use it if BEATENGINE_TEST is defined.");
}

void Game::InitViews() {
	Logger::AddDebug(typeid(Game), "Initializing views...");
}

void Game::InitSystems() {
    Logger::AddDebug(typeid(Game), "Initializing systems...");
}

void Game::InitAssets() {
	Logger::AddDebug(typeid(Game), "Initializing assets...");
}

void Game::InitWindow() {
	Logger::AddDebug(typeid(Game), "Initializing window...");

    m_State.GetGraphicsMgr().Init();

	auto settings = m_State.GetSettingsMgr().GetSettings(typeid(GameSettings));
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
        m_Context.GFlags |= GameFlags_Fullscreen;

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
