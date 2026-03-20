#include "GameLayer.h"
#include "BeatEngine/View/ViewLayer.h"
#include <memory>

GameLayer::GameLayer(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state)
    : ViewLayer(typeid(GameLayer), context, state)
{}
