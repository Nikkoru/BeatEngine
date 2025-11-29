#define BEATENGINE_TEST

#include <BeatEngine/Game.h>
#include <BeatEngine/Enum/AssetType.h>
#include <BeatEngine/Settings/GameSettings.h>

#include "view/view.h"
#include "system/system.h"
int main() {
	Game game;

	game.RegisterView<TestView>();
	game.RegisterSystem<SettingsSystemTest>();

	game.PreloadSettings();

	auto settings = game.GetSettings<GameSettings>();

	game.GetWindow()->setFramerateLimit(settings->FpsLimit);
	game.GetWindow()->setSize(settings->WindowSize);

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

	game.SaveSettings();

	game.Run();
}