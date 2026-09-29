#include "game.hpp"
#include "BeatEngine/Application.hpp"
#include "BeatEngine/Enum/AppFlags.hpp"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "layer/globalLayer.h"
#include "system/system.h"
#include "view/gameView.h"
#include "view/view.h"
#include <memory>

Game::Game(int argc, char** argv)
    : m_Argc(argc)
    , m_Argv(argv)
    , Application("BeatEngineTest") {

    m_Context.AddAFlags(AppFlags_ImGui);
    m_Context.AddAFlags(AppFlags_ImGuiDocking);

    auto window = std::make_shared<SDLWindow>();
    window->SetInitFlags(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    window->SetWindowFlags(SDL_WINDOW_RESIZABLE);
    window->SetTitle("BE");
    window->SetSize({ 1280, 720 });

    m_Renderer = std::make_unique<VulkanRenderer>(&m_Context, window);
    m_Renderer->SetWindow(window);

    std::vector<std::filesystem::path> paths;

    m_State.GetViewMgr().RegisterView<TestView>();
    m_State.GetViewMgr().RegisterView<GameView>();
    m_State.GetSystemMgr().RegisterSystem<SettingsSystemTest>();

    if (m_Argc >= 2) {
        auto index = std::stoi(m_Argv[1]);
        
        // m_Renderer->SetDeviceIndex(index);
    }
    if (m_Argc >= 3) {
        if (!std::filesystem::exists(m_Argv[2])) {
            Logger::AddCritical("\"{}\" must be a valid path that contains .mp3 files", m_Argv[2]);
            THROW_RUNTIME_ERROR("bleh");
        }

        for (const auto& entry : fs::directory_iterator(m_Argv[2])) {
            if (entry.path().extension() == ".mp3" || entry.path().extension() == ".flac") { 
                paths.emplace_back(entry.path());
            }
        }
    }
    else {
#ifndef _WIN32
        paths = {
            "assets/music/audio.mp3", 
            "assets/music/eurobeat.mp3", 
            "assets/music/kiby-aqua.mp3", 
            "assets/music/kiby-star.mp3", 
            "assets/music/remix7.mp3", 
            "assets/music/reverse-mountain.mp3", 
            "assets/music/test-music.mp3", 
            "assets/music/audio.mp3",
            "assets/music/abstraction.mp3"
        };
#else
        paths = {
            "assets\\music\\audio.mp3", 
            "assets\\music\\eurobeat.mp3", 
            "assets\\music\\kiby-aqua.mp3", 
            "assets\\music\\kiby-star.mp3", 
            "assets\\music\\remix7.mp3", 
            "assets\\music\\reverse-mountain.mp3", 
            "assets\\music\\test-music.mp3", 
            "assets\\music\\audio.mp3",
            "assets\\music\\abstraction.mp3"
        };
#endif
    }

    m_State.GetAssetMgr().BulkLoad({
		{
			typeid(Font),
			{
				"assets/fonts/main-font.ttf"
			}
		},
		{
			typeid(Sound),
			{
				"assets/sounds/test-sound.mp3"
			}
		},
        {
            typeid(AudioStream),
            paths
        },
        {
            typeid(Shader),
            {
                "assets/shaders/shader.frag"
                "assets/shaders/shader.vert"
            }
        },
	});

	m_GlobalLayers.AttachLayer<GlobalTestLayerUI>(&m_Context, &m_State);
}
