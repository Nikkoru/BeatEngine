#include "BeatEngine/Application.hpp"

#include "BeatEngine/Events/AppEvent.hpp"

#include "BeatEngine/Graphics/BaseWindow.h"

#include "BeatEngine/Settings/AppDebugSettings.hpp"
#include "BeatEngine/Settings/AppSettings.hpp"

#include "BeatEngine/Signals/AppSignals.hpp"
#include "BeatEngine/Signals/ViewSignals.h"
#include "BeatEngine/Util/Profiler.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/SignalManager.h"

Application::Application(const std::string& name): m_Context(name) {
    m_State.PrepareManagers(&m_Context);
#ifdef BEATENGINE_DEBUG
    // Logger::PrintDebug(true);
    Logger::AddInfo("", "Debug Build");
    m_Context.EFlags |= EnvFlags_Debug;
#endif // BEATENGINE_DEBUG
#ifdef BEATENGINE_TEST
    Logger::AddInfo("", "This is a Test Build");
    m_MainContext->EFlags |= EnvFlags_TestBuild;
#endif // BEATENGINE_TEST
}

void Application::Init() {
    Logger::AddInfo(typeid(Application), "Initializing Application");
    
    CustomInit();

    _InitSettings();
	_InitAudio();
	_InitSystems();
	_InitGraphics();
	_InitAssets();
	_InitUI();
	_InitViews();
	_InitKeybinds();

	_SubscribeToAppEvent();
	_SubscribeToAppSignals();
}

void Application::Uninit() {
    Logger::AddInfo(typeid(Application), "Application in shutdown");

    // m_State->KeybindsMgr->Uninit();
    m_State.GetViewMgr().Uninit();
    m_State.GetUIMgr().Uninit();
    m_State.GetAssetMgr().Uninit();
    m_State.GetSystemMgr().StopSystems();
    m_State.GetAudioMgr().Uninit();
    m_State.GetGraphicsMgr().Close();
    // m_SettingsMgr->Uninit();

    CustomUninit();
}

void Application::Run() {
	Logger::AddInfo(typeid(Application), "Application started!");
    auto& viewMgr = m_State.GetViewMgr();
    auto& graphicsMgr = m_State.GetGraphicsMgr();

    if (!viewMgr.HasActiveViews())
        viewMgr.Push(viewMgr.MainView);

    while (graphicsMgr.IsOpen()) {
        while (auto event = graphicsMgr.PollEvent()) {
         if (event->Is<AppExitingEvent>()) {
                graphicsMgr.Close();
                break;
            }
            m_GlobalLayers.OnEvent(event);
            if (viewMgr.OnEvent(event)) {
                break;
            }
        }

        Update();
        Draw();
        Display();
    }

    Uninit();
}

void Application::Update() {
    Profiler::StartProfile({ typeid(Application), "Update" }, { .0f, 1.0f, .0f, 1.0f });
    auto& graphicsMgr = m_State.GetGraphicsMgr();

    m_Context.WindowSize = graphicsMgr.GetWindow()->GetSize();
    //
    // if (m_Context->GFlags & ApplicationFlags_CursorChanged) {
    //     m_Window->setMouseCursor(m_Cursor);
    //     m_Context->GFlags &= ~ApplicationFlags_CursorChanged;
    // }

	auto sfDelta = m_MainClock.GetAndReset();
	auto deltaTime = sfDelta.AsSeconds();

	if (!this->m_State.GetViewMgr().OnUpdate(deltaTime)) {
        Uninit();
		return;
	}

    graphicsMgr.Update();
	
    m_State.GetSystemMgr().Update(deltaTime);
	m_GlobalLayers.OnUpdate(deltaTime);
    m_State.GetUIMgr().Update(deltaTime);

    m_LastDelta = deltaTime;

    Profiler::EndProfile({ typeid(Application), "Update" });
}

void Application::Display() {
    Profiler::StartProfile({ typeid(Application), "Display" }, { 1.0f, .0f, .0f, 1.0f });
    if (m_Context.GFlags & AppFlags_ImGui && m_Context.GFlags & AppFlags_DrawDebugInfo) {
        // DrawImGuiDebug();
    }

    m_State.GetGraphicsMgr().Clear();
	m_State.GetGraphicsMgr().Display();

    Profiler::EndProfile({ typeid(Application), "Display" });
}

void Application::Draw() {
    Profiler::StartProfile({ typeid(Application), "Draw" }, { .0f, .0f, 1.0f, 1.0f });
    m_State.GetGraphicsMgr().Render();

    if (m_Context.GFlags & AppFlags_DebugDock && 
        m_Context.GFlags & AppFlags_ImGui &&
        m_Context.GFlags & AppFlags_ImGuiDocking &&
        m_Context.GFlags & AppFlags_DrawDebugInfo) {
        auto io = ImGui::GetIO();
        auto viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

        // ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiWindowFlags_NoDocking;
        ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
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
	// m_Running = m_State.GetViewMgr().OnDraw();

	m_GlobalLayers.Draw(m_State.GetGraphicsMgr());
    m_State.GetUIMgr().OnDraw();

    Profiler::EndProfile({ typeid(Application), "Draw" });
}

void Application::_InitSettings() {
	Logger::AddDebug(typeid(Application), "Initializing settings...");
    
	m_State.GetSettingsMgr().RegisterSettingsData<AppSettings>();
    // m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);

#ifdef BEATENGINE_DEBUG
    m_State.GetSettingsMgr().RegisterSettingsData<AppDebugSettings>();
    m_State.GetSettingsMgr().ReadConfig("debug.ini");
#endif
}

void Application::_InitUI() {
	Logger::AddDebug(typeid(Application), "Initializing UI...");
}

void Application::_InitAudio() {
	Logger::AddDebug(typeid(Application), "Initializing audio...");

    m_State.GetAudioMgr().Init();
}

void Application::_InitViews() {
	Logger::AddDebug(typeid(Application), "Initializing views...");

    m_State.GetViewMgr().Init();
}

void Application::_InitSystems() {
    Logger::AddDebug(typeid(Application), "Initializing systems...");
}

void Application::_InitAssets() {
	Logger::AddDebug(typeid(Application), "Initializing assets...");

    m_State.GetAssetMgr().Init();
}

void Application::_InitGraphics() {
	Logger::AddDebug(typeid(Application), "Initializing window...");

	auto settings = m_State.GetSettingsMgr().GetSettings(typeid(AppSettings));
	auto gameSettings = std::static_pointer_cast<AppSettings>(settings);

    m_State.GetGraphicsMgr().SetWindowFullscreen(gameSettings->WindowFullScreen);
    m_State.GetGraphicsMgr().SetFramerateLimit(gameSettings->FpsLimit);
    m_State.GetGraphicsMgr().Init();

    if (gameSettings->WindowFullScreen) {
        m_Context.GFlags |= AppFlags_Fullscreen;
    }

    m_Context.WindowSize = m_State.GetGraphicsMgr().GetWindow()->GetSize();
}

void Application::_InitKeybinds() {
	Logger::AddDebug(typeid(Application), "Initializing keybinds... (not really)");
}

void Application::_SubscribeToAppEvent() {
	Logger::AddDebug(typeid(Application), "Subscribing to game events...");

    EventManager::GetInstance()->Subscribe<AppSettingsChangedEvent>([this](std::shared_ptr<Base::Event>) {
        
        auto settings = std::static_pointer_cast<AppSettings>(m_State.GetSettingsMgr().GetSettings(typeid(AppSettings)));
        
        bool curFullscreen = m_Context.GFlags & AppFlags_Fullscreen;

        if (settings->WindowFullScreen != curFullscreen) {
            // m_Window->close();
            // delete m_Window;
            // if (settings->WindowFullScreen)
            //     this->m_Window = new sf::RenderWindow(
            //         sf::VideoMode{}, 
            //         "BeatEngine Application",
            //         sf::Style::Default,
            //         sf::State::Fullscreen 
            //     );
            // else {
            //     this->m_Window = new sf::RenderWindow(
            //         sf::VideoMode(settings->WindowSize), 
            //         "BeatEngine Application",
            //         sf::Style::Default,
            //         sf::State::Windowed
            //     );
            // }

            if (settings->WindowFullScreen)
                m_Context.GFlags |= AppFlags_Fullscreen;
            else if (m_Context.GFlags & AppFlags_Fullscreen)
                m_Context.GFlags &= ~AppFlags_Fullscreen;
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

void Application::_SubscribeToAppSignals() {
	Logger::AddDebug(typeid(Application), "Subscribing to game signals...");

	SignalManager::GetInstance()->RegisterCallback<ViewAddGlobalLayerSignal>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
		auto signal = std::static_pointer_cast<ViewAddGlobalLayerSignal>(sig);
		this->m_GlobalLayers.AttachLayer(signal->Layer);

		signal->Layer = nullptr;
	});

    SignalManager::GetInstance()->RegisterCallback<AppExitSignal>(typeid(Application), [this](const std::shared_ptr<Base::Signal>) {
        EventManager::GetInstance()->Send(std::make_shared<AppExitingEvent>());
        // m_Running = false;
    });

    // SignalManager::GetInstance()->RegisterCallback<AppChangeCursorSignal>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
    //     auto gameSig = std::static_pointer_cast<AppChangeCursorSignal>(sig);
    //     // m_Cursor = sf::Cursor::createFromSystem(gameSig->NewCursor).value();
    //     m_Context->GFlags |= AppFlags_CursorChanged;
    // });

    SignalManager::GetInstance()->RegisterCallback<AppToggleImGui>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<AppToggleImGui>(sig);
        if (m_Context.GFlags & AppFlags_ImGui)
            m_Context.GFlags &= ~AppFlags_ImGui;
        else
            m_Context.GFlags |= AppFlags_ImGui;
    });

    SignalManager::GetInstance()->RegisterCallback<AppAddFlags>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<AppAddFlags>(sig);
        // this->SetFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<AppRemoveFlags>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<AppRemoveFlags>(sig);
        // this->RemoveFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<ViewAddFlags>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<ViewAddFlags>(sig);
        this->m_Context.VFlags |= gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<ViewRemoveFlags>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<ViewAddFlags>(sig);
        this->m_Context.VFlags &= ~gameSig->Flags;
    });

    SignalManager::GetInstance()->RegisterCallback<AppToggleDrawingDebugInfo>(typeid(Application), [this](const std::shared_ptr<Base::Signal>) {
            if (m_Context.GFlags & AppFlags_DrawDebugInfo)
                m_Context.GFlags &= ~AppFlags_DrawDebugInfo;
            else
                m_Context.GFlags |= AppFlags_DrawDebugInfo;
    });
}
