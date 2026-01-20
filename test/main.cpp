#define BEATENGINE_TEST

#include <BeatEngine/Game.h>
#include <BeatEngine/Enum/AssetType.h>
#include <BeatEngine/Settings/GameSettings.h>

#include "view/view.h"
#include "system/system.h"
#include "layer/globalLayer.h"

int main() {
	Game game;

	game.RegisterView<TestView>();
	game.RegisterSystem<SettingsSystemTest>();

    game.UseImGui(true);
    game.UseImGuiDocking(true);

	game.PreloadSettings();

	auto settings = game.GetSettings<GameSettings>();

	game.SetWindowTitle("Now you can change the title!");

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

	game.AddGlobalLayer<GlobalTestLayerUI>();
    auto layer = game.GetGlobalLayer<GlobalTestLayerUI>();

	game.SaveSettings();

	game.Run();
}
