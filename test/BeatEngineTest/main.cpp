#include "game.hpp"

int main(int argc, char** argv) {
//    game.UseImGui(true);
//    game.UseImGuiDocking(true);

    Game game{ argc, argv };
    game.Init();
    game.Run();
}
