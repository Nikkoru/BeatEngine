
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <filesystem>
#include <memory>
#include <BeatEngine/Logger.h>
// #include <BeatEngine/Renderers/OpenGL/Renderer.h>
#include <BeatEngine/Game.h>
#include <BeatEngine/Enum/AssetType.h>
#include <BeatEngine/Settings/GameSettings.h>
#include <BeatEngine/Renderers/Vulkan/Renderer.h>
#include <BeatEngine/Windows/SDL/Window.h>

#include "view/gameView.h"
#include "view/view.h"
#include "system/system.h"
#include "layer/globalLayer.h"

int main(int argc, char** argv) {
    std::vector<std::filesystem::path> paths;
    
    if (argc >= 2) {
        if (!std::filesystem::exists(argv[1])) {
            Logger::AddCritical("\"{}\" must be a valid path that contains .mp3 files", argv[1]);
            return 1;
        }

        for (const auto& entry : fs::directory_iterator(argv[1])) {
            if (entry.path().extension() == ".mp3" || entry.path().extension() == ".flac") { 
                paths.emplace_back(entry.path());
            }
        }
    }
    else {
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
    }

	Game game;

	game.RegisterView<TestView>();
	game.RegisterView<GameView>();

	game.RegisterSystem<SettingsSystemTest>();

    game.UseImGui(true);
    game.UseImGuiDocking(true);

    auto renderer = std::make_shared<VulkanRenderer>();
    auto window = std::make_shared<SDLWindow>();
    window->SetInitFlags(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    window->SetWindowFlags(SDL_WINDOW_RESIZABLE);

    renderer->SetWindow(window);

    game.SetRenderer(renderer);

	game.SetWindowTitle("Now you can change the title!");
    game.SetWindowSize({ 1280, 720 });

	game.LoadGlobalAssets({
		{
			AssetType::Font,
			{
				"assets/fonts/main-font.ttf"
			}
		},
		{
			AssetType::Sound,
			{
				"assets/sounds/test-sound.mp3"
			}
		},
        {
            AssetType::AudioStream,
            paths
        },
        // {
        //     AssetType::FragmentShader,
        //     {
        //         "assets/shaders/shader.frag"
        //     }
        // },
        // {
        //     AssetType::VertexShader,
        //     {
        //         "assets/shaders/shader.vert"
        //     }
        // }
        // {
        //     AssetType::ComputeShader,
        //     {
        //         "/home/niko/Projects/BeatEngine/assets/shaders/gradient.comp"
        //     }
        // }
	});

    game.Init();
	game.AddGlobalLayer<GlobalTestLayerUI>();

	game.Run();
}
