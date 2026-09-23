#include "GameLayer.h"
#include "BeatEngine/View/ViewLayer.h"
#include <memory>

GameLayer::GameLayer(std::shared_ptr<AppContext> context, std::shared_ptr<AppState> state)
    : ViewLayer(typeid(GameLayer), context, state)
{}
