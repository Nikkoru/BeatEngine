#define BEATENGINE_TEST

#include <BeatEngine/Game.h>
#include <BeatEngine/Enum/AssetType.h>

#include "view/view.h"
#include "system/system.h"

int main() {
	Game game;

	game.RegisterView<TestView>();
	game.RegisterSystem<SettingsSystemTest>();



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
		}
	});

	game.Run();
}