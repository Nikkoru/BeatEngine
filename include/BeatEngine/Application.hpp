#pragma once

#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/AppState.hpp"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/View/ViewLayerStack.h"

class Application {
protected:
    Clock m_MainClock{};
    float m_LastDelta{};

    ViewLayerStack m_GlobalLayers{};
    AppContext m_Context{};
    AppState m_State{};

    bool m_Running = false;

	std::filesystem::path m_SettingsPath = "config.ini";
public:
    Application(const std::string& name = "BeatEngine Program");
    virtual ~Application() = default;
public:
    virtual void Init();
    virtual void Uninit();
    virtual void Run();

    virtual void Update();
    virtual void Display();
    virtual void Draw();
private:
    void _InitSettings();
    void _InitUI();
    void _InitAudio();
    void _InitViews();
    void _InitSystems();
    void _InitAssets();
    void _InitGraphics();
    void _InitKeybinds();
    void _SubscribeToAppEvent();
    void _SubscribeToAppSignals();
};
