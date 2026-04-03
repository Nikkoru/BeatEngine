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
    // Logger::GetInstance()->SetLogLevel();

	Game game;

	game.RegisterView<TestView>();
	game.RegisterView<GameView>();

	game.RegisterSystem<SettingsSystemTest>();

    game.UseImGui(true);
    game.UseImGuiDocking(true);

    // game.SetRenderer<OpenGLRenderer>();

	// game.SetWindowTitle("Now you can change the title!");

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
	});

    game.Initialize();
	game.AddGlobalLayer<GlobalTestLayerUI>();

	game.Run();
}
