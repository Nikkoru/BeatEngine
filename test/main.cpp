#include <filesystem>
#define BEATENGINE_TEST

#include "BeatEngine/Logger.h"
#include "view/gameView.h"
#include <BeatEngine/Game.h>
#include <BeatEngine/Enum/AssetType.h>
#include <BeatEngine/Settings/GameSettings.h>

// #include <BeatEngine/Renderers/OpenGL.h>

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
            if (entry.path().extension() == ".mp3") { 
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

    // game.SetRenderer<OpenGLRenderer>();

	// game.SetWindowTitle("Now you can change the title!");
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
        {
            AssetType::ComputeShader,
            {
                "assets/shaders/gradient.comp"
            }
        }
	});

    game.Initialize();
	game.AddGlobalLayer<GlobalTestLayerUI>();

	game.Run();
}
