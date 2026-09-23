#include "BeatEngine/Application.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <imgui.h>
#include <imgui_internal.h>
#include <limits>
#include <memory>

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/AssetType.h"
#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Logger.h"

#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Manager/SettingsManager.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/UIManager.h"
#include "BeatEngine/Manager/ViewManager.h"

#include "BeatEngine/Settings/AppSettings.hpp"
#include "BeatEngine/Settings/AppDebugSettings.hpp"

#include "BeatEngine/Signals/AppSignals.hpp"
#include "BeatEngine/Signals/ViewSignals.h"

#include "BeatEngine/Events/AppEvent.hpp"
#include "BeatEngine/System/Time.h"

#include "BeatEngine/Util/CountedArray.h"
#include "BeatEngine/Util/Profiler.h"

#include "version.h"

Application::Application() : Application("BeatEngine Game") {
}

Application::Application(const std::string name): m_Context(name) {
    m_State.PrepareManagers(&m_Context);
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

Application::~Application() {
}

void Application::Run() {
    m_Running = true;
	Logger::AddInfo(typeid(Application), "Application started!");

	if (!m_State.GetViewMgr().HasActiveViews())
		m_State.GetViewMgr().Push(m_State.GetViewMgr().MainView);


	if (m_Context.GFlags & AppFlags_Preload) {
		m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);
		ApplyBaseSettings();
	}

	while (m_State.GetGraphicsMgr().IsOpen() && m_Running) {
		while (auto event = m_State.GetGraphicsMgr().PollEvent()) {
            if (event->Is<AppExitingEvent>()) {
                m_Running = false;
                break;
            }
            m_GlobalLayers.OnEvent(event);
            if (!m_State.GetViewMgr().OnEvent(event)) {
                m_Running = false;
                break;
            }
        }
        if (!m_Running) break;

        this->Update();

        this->Draw();
        this->Display();
	}
    Uninit();
}

void Application::Init() {
    Logger::AddInfo(typeid(Application), "Initializing Application");

    InitSettings();
	InitAudio();
	InitSystems();
	InitWindow();
	InitAssets();
	InitUI();
	InitViews();
	InitKeybinds();

	SubscribeToApplicationEvent();
	SubscribeToApplicationSignals();
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
    m_Running = false;
}

void Application::SetRenderer(std::shared_ptr<Renderer> renderer) {
    m_State.GetGraphicsMgr().MakeRenderer(renderer);
}

void Application::UseImGui(bool show) {
    if (show)
        m_Context.GFlags |= AppFlags_ImGui;
    else
        m_Context.GFlags &= ~AppFlags_ImGui;
}

void Application::UseImGuiDocking(bool docking) {
    if (docking)
        m_Context.GFlags |= AppFlags_ImGuiDocking;
    else
        m_Context.GFlags &= ~AppFlags_ImGuiDocking;
}

void Application::SetWindowSize(Vector2u size) {
    if (m_State.GetSettingsMgr().HasSettings<AppSettings>()) {
        auto settings = std::static_pointer_cast<AppSettings>(m_State.GetSettingsMgr().GetSettings(typeid(AppSettings)));
        settings->WindowSize = size;
    }
    m_State.GetGraphicsMgr().SetWindowSize(size);
}

void Application::SetWindowTitle(std::string title) {
    m_State.GetGraphicsMgr().SetWindowTitle(title);
}

void Application::PreloadSettings() {
    m_Context.GFlags |= AppFlags_Preload;

	m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);
}

void Application::SaveSettings() {
	m_State.GetSettingsMgr().WriteConfig(m_SettingsPath);
}

void Application::SetConfigPath(std::filesystem::path path) {
	this->m_SettingsPath = path;
}

void Application::SetFlags(AppFlags flags) {
    this->m_Context.GFlags |= flags;
}

void Application::RemoveFlags(AppFlags flags) {
    this->m_Context.GFlags &= ~flags;
}

void Application::DrawImGuiDebug() {
    static bool editFlags = false;
    static bool profWindow = false;

    static bool drawAudioMgr = false;
    static bool drawAssetMgr = false;
    static bool drawEntityMgr = false;
    static bool drawEventMgr = false;
    static bool drawGraphicsMgr = false;
    static bool drawSettingsMgr = false;
    static bool drawSignalMgr = false;
    static bool drawSystemMgr = false;
    static bool drawUIMgr = false;
    static bool drawViewMgr = false;

    ImGui::Begin("BeatEngine Application Debug Window", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Environment")) {
            bool dockingStatus = (m_Context.GFlags & AppFlags_DebugDock);
            if (ImGui::MenuItem("Enable Docking", NULL, dockingStatus)) {
                !dockingStatus ? m_Context.GFlags |= AppFlags_DebugDock :
                            m_Context.GFlags &= ~AppFlags_DebugDock;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Behaviour")) {
            if (ImGui::MenuItem("Profiler Window", NULL, profWindow))
                profWindow = !profWindow;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Managers")) {
            if (ImGui::MenuItem("AudioManager window", NULL, drawAudioMgr))
                drawAudioMgr = !drawAudioMgr;
            if (ImGui::MenuItem("AssetManager window", NULL, drawAssetMgr))
                drawAssetMgr = !drawAssetMgr;
            if (ImGui::MenuItem("EntityManager window", NULL, drawEntityMgr))
                drawEntityMgr = !drawEntityMgr;
            if (ImGui::MenuItem("EventManager window", NULL, drawEventMgr))
                drawEventMgr = !drawEventMgr;
            if (ImGui::MenuItem("GraphicsManager window", NULL, drawGraphicsMgr))
                drawGraphicsMgr = !drawGraphicsMgr;
            if (ImGui::MenuItem("SettingsManager window", NULL, drawSettingsMgr))
                drawSettingsMgr = !drawSettingsMgr;
            if (ImGui::MenuItem("SignalManager window", NULL, drawSignalMgr))
                drawSignalMgr = !drawSignalMgr;
            if (ImGui::MenuItem("SystemManager window", NULL, drawSystemMgr))
                drawSystemMgr = !drawSystemMgr;
            if (ImGui::MenuItem("UIManager window", NULL, drawUIMgr))
                drawUIMgr = !drawUIMgr;
            if (ImGui::MenuItem("ViewManager window", NULL, drawViewMgr))
                drawViewMgr = !drawViewMgr;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::BeginTabBar("AppActionBar")) {
        if (ImGui::BeginTabItem("Flags")) {
            bool imguiToggle = m_Context.GFlags & AppFlags_ImGui;
            bool imguiDockingToggle = m_Context.GFlags & AppFlags_ImGuiDocking;
            bool runningToggle = m_Context.GFlags & AppFlags_Running;
            bool preloadToggle = m_Context.GFlags & AppFlags_Preload;
            bool fullscreenToggle = m_Context.GFlags & AppFlags_Fullscreen;
            bool cursorChangedToggle = m_Context.GFlags & AppFlags_CursorChanged;
            bool disableKeysToggle = m_Context.GFlags & AppFlags_DisableKeyPressEvents;
            bool drawDebugToggle = m_Context.GFlags & AppFlags_DrawDebugInfo;
            bool drawDockToggle = m_Context.GFlags & AppFlags_DebugDock;

            bool viewDisableKeyToggle = m_Context.VFlags & ViewFlags_DisableKeys;

            bool envDebugToggle = m_Context.EFlags & EnvFlags_Debug;
            bool envTestToggle = m_Context.EFlags & EnvFlags_TestBuild;
            
            ImGui::Text("AppFlags: %#.8x", m_Context.GFlags);
            if (!editFlags)
                ImGui::BeginDisabled();
            ImGui::Checkbox("AppFlags_ImGui", &imguiToggle);
            ImGui::Checkbox("AppFlags_ImGuiDocking", &imguiDockingToggle);
            ImGui::Checkbox("AppFlags_Running", &runningToggle);
            ImGui::Checkbox("AppFlags_Preload", &preloadToggle);
            ImGui::Checkbox("AppFlags_Fullscreen", &fullscreenToggle);
            ImGui::Checkbox("AppFlags_CursorChanged", &cursorChangedToggle);
            ImGui::Checkbox("AppFlags_DisableKeyPressEvents", &disableKeysToggle);
            ImGui::Checkbox("AppFlags_DrawDebugInfo", &drawDebugToggle);
            ImGui::Checkbox("AppFlags_DebugDock", &drawDockToggle);
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

            if (!imguiToggle && m_Context.GFlags & AppFlags_ImGui)
                m_Context.GFlags &= AppFlags_ImGui;
            else if (imguiToggle && !(m_Context.GFlags &AppFlags_ImGui))
                m_Context.GFlags |= ~AppFlags_ImGui;

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Actions")) {
            if (ImGui::Button("Exit"))
                SignalManager::GetInstance()->Send(std::make_shared<AppExitSignal>());
            ImGui::Checkbox("Allow editing flags", &editFlags);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Status")) {
            static CountedArray<float, 50> deltas;
            if (deltas.Full()) {
                auto data = deltas.Data();
                std::move(data + 1, data + 50, data);
                data[50 - 1] = LastDelta;
            }
            else deltas.Add(LastDelta);
            
            float avgDelta{};
            float minDelta{ std::numeric_limits<float>::max() };
            float maxDelta{ std::numeric_limits<float>::min() };
            for (const auto& delta : deltas) {
                avgDelta += delta;
                if (delta < minDelta)
                    minDelta = delta;
                if (delta > maxDelta)
                    maxDelta = delta;
            }
            avgDelta /= deltas.UsedSize();

            ImGui::Text("Raw Delta: %.3f (%.1f ms)", LastDelta, LastDelta * 1000);
            ImGui::SameLine();
            ImGui::Text("Raw FPS: %.2f", 1 / LastDelta);
            ImGui::Text("Avg Delta: %.3f (%.1f ms)", avgDelta, avgDelta * 1000);
            ImGui::SameLine();
            ImGui::Text("Avg FPS: %.2f", 1 / avgDelta);
            ImGui::Text("Min Delta: %.3f (%.1f ms)", minDelta, minDelta * 1000);
            ImGui::SameLine();
            ImGui::Text("Max FPS: %.2f", 1 / minDelta);
            ImGui::Text("Max Delta: %.3f (%.1f ms)", maxDelta, maxDelta * 1000);
            ImGui::SameLine();
            ImGui::Text("Min FPS: %.2f", 1 / maxDelta);

            ImGui::Separator();

            ImGui::Text("Build date: %s", __DATE__);
            ImGui::Text("Build time: %s", __TIME__);
            ImGui::Text("Build commit: %s", BEATENGINE_COMMIT_ID_BUILD);

            ImGui::EndTabItem();
        }
        if (profWindow) {
            ImGui::Begin("Profiler", &profWindow);
            Profiler::DrawHistogram(ImGui::GetContentRegionAvail());
            ImGui::End();
        }
        else if (ImGui::BeginTabItem("Profiler")) {
            Profiler::DrawHistogram(ImGui::GetContentRegionAvail());

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Context")) {
            auto size = m_Context.WindowSize;
            ImGui::Text("WindowSize: (X: %u Y: %u)", size.X, size.Y);
            ImGui::Text("ActiveView: %s", m_Context.ActiveView.name());
            ImGui::Text("ProgramName: %s", m_Context.ProgramName.c_str());

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

    if (drawAudioMgr)
        m_State.GetAudioMgr().ShowImGuiDebugWindow();
    if (drawAssetMgr)
        m_State.GetAssetMgr().ShowImGuiDebugWindow();
    // if (drawEntityMgr)
    //     m_State.GetEntityMgr().ShowImGuiDebugWindow();
    // if (drawEventMgr)
    //     EventManager::GetInstance()->ShowImGuiDebugWindow();
    if (drawGraphicsMgr)
        m_State.GetGraphicsMgr().ShowImGuiDebugWindow();
    if (drawSettingsMgr)
        m_State.GetSettingsMgr().ShowImGuiDebugWindow();
    // if (drawSignalMgr)
    //     SignalManager::GetInstance()->ShowImGuiDebugWindow();
    // if (drawSystemMgr)
    //     m_State.GetSystemMgr().ShowImGuiDebugWindow();
    if (drawUIMgr)
        m_State.GetUIMgr().ShowImGuiDebugWindow();
    if (drawViewMgr)
        m_State.GetViewMgr().ShowImGuiDebugWindow();
}

void Application::LoadGlobalAssets(std::unordered_map<AssetType, std::vector<std::filesystem::path>> globalAssets) {
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

    Logger::AddDebug(typeid(Application), "Preloaded {} assets", assets);
}

void Application::Display() {
    Profiler::StartProfile({ typeid(Application), "Display" }, { 1.0f, .0f, .0f, 1.0f });
    if (m_Context.GFlags & AppFlags_ImGui && m_Context.GFlags & AppFlags_DrawDebugInfo) {
        DrawImGuiDebug();
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

	m_Running = m_State.GetViewMgr().OnDraw();

	m_GlobalLayers.Draw(m_State.GetGraphicsMgr());
    m_State.GetUIMgr().OnDraw();

    Profiler::EndProfile({ typeid(Application), "Draw" });
}

void Application::Update() {
    Profiler::StartProfile({ typeid(Application), "Update" }, { .0f, 1.0f, .0f, 1.0f });
    m_Context.WindowSize = m_State.GetGraphicsMgr().GetWindow()->GetSize();
    //
    // if (m_Context->GFlags & ApplicationFlags_CursorChanged) {
    //     m_Window->setMouseCursor(m_Cursor);
    //     m_Context->GFlags &= ~ApplicationFlags_CursorChanged;
    // }

	auto sfDelta = m_Clock.GetAndReset();
	auto deltaTime = sfDelta.AsSeconds();

	if (!this->m_State.GetViewMgr().OnUpdate(deltaTime)) {
        Uninit();
		return;
	}

    m_State.GetGraphicsMgr().Update();
	
    this->m_State.GetSystemMgr().Update(deltaTime);
	m_GlobalLayers.OnUpdate(deltaTime);
    m_State.GetUIMgr().Update(deltaTime);

    LastDelta = deltaTime;

    Profiler::EndProfile({ typeid(Application), "Update" });
}

void Application::ApplyBaseSettings() {
	auto gameSettings = std::static_pointer_cast<AppSettings>(m_State.GetSettingsMgr().GetSettings(typeid(AppSettings)));
    auto window = m_State.GetGraphicsMgr().GetWindow();

    m_State.GetGraphicsMgr().SetFramerateLimit(gameSettings->FpsLimit);

	window->SetSize(gameSettings->WindowSize);
	if (gameSettings->WindowPosition != Vector2i(-1, -1))
		window->SetPosition(gameSettings->WindowPosition);
}

void Application::InitSettings() {
	Logger::AddDebug(typeid(Application), "Initializing settings...");
    
	m_State.GetSettingsMgr().RegisterSettingsData<AppSettings>();
    m_State.GetSettingsMgr().ReadConfig(m_SettingsPath);

#ifdef BEATENGINE_DEBUG
    m_State.GetSettingsMgr().RegisterSettingsData<AppDebugSettings>();
    m_State.GetSettingsMgr().ReadConfig("debug.ini");
#endif
}

void Application::InitUI() {
	Logger::AddDebug(typeid(Application), "Initializing UI...");
}

void Application::InitAudio() {
	Logger::AddDebug(typeid(Application), "Initializing audio...");

    m_State.GetAudioMgr().Init();
}

void Application::InitViews() {
	Logger::AddDebug(typeid(Application), "Initializing views...");

    m_State.GetViewMgr().Init();
}

void Application::InitSystems() {
    Logger::AddDebug(typeid(Application), "Initializing systems...");
}

void Application::InitAssets() {
	Logger::AddDebug(typeid(Application), "Initializing assets...");

    m_State.GetAssetMgr().Init();
}

void Application::InitWindow() {
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

void Application::InitKeybinds() {
	Logger::AddDebug(typeid(Application), "Initializing keybinds... (not really)");
}

void Application::SubscribeToApplicationEvent() {
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

void Application::SubscribeToApplicationSignals() {
	Logger::AddDebug(typeid(Application), "Subscribing to game signals...");

	SignalManager::GetInstance()->RegisterCallback<ViewAddGlobalLayerSignal>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
		auto signal = std::static_pointer_cast<ViewAddGlobalLayerSignal>(sig);
		this->m_GlobalLayers.AttachLayer(signal->Layer);

		signal->Layer = nullptr;
	});

    SignalManager::GetInstance()->RegisterCallback<AppExitSignal>(typeid(Application), [this](const std::shared_ptr<Base::Signal>) {
        EventManager::GetInstance()->Send(std::make_shared<AppExitingEvent>());
        m_Running = false;
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
        this->SetFlags(gameSig->Flags);
    });

    SignalManager::GetInstance()->RegisterCallback<AppRemoveFlags>(typeid(Application), [this](const std::shared_ptr<Base::Signal> sig) {
        auto gameSig = std::static_pointer_cast<AppRemoveFlags>(sig);
        this->RemoveFlags(gameSig->Flags);
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
