#pragma once

#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Enum/ViewFlags.h"
#include <SFML/System/Vector2.hpp>
#include <typeindex>

class GameContext {
public:
    GameFlags GFlags = GameFlags_None;
    ViewFlags VFlags = ViewFlags_None;
    sf::Vector2u WindowSize = {};
    std::type_index ActiveView = typeid(nullptr);
public:
    GameContext() = default;
};
