#pragma once

#include "BeatEngine/Base/Event.h"

#include <SFML/System/Vector2.hpp>

class GameSettingsChanged : public Base::Event {
public:
    GameSettingsChanged() : Base::Event(typeid(GameSettingsChanged)) {}
};

class GameResized : public Base::Event {
public:
    sf::Vector2u Size; 
public:
    GameResized(sf::Vector2u newSize) : Base::Event(typeid(GameResized)), Size(newSize) {} 
};
