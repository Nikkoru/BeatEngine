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

int main() {
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
