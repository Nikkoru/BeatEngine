#pragma once

#include "BeatEngine/Base/Signal.h"
#include <SFML/Window/Cursor.hpp>

class GameChangeCursorSignal : public Base::Signal {
public:
    sf::Cursor::Type NewCursor;
public:
    GameChangeCursorSignal(sf::Cursor::Type cursorType) 
        : Base::Signal(typeid(GameChangeCursorSignal)), NewCursor(cursorType) {}
};

class GameExitSignal : public Base::Signal {    
public:
    GameExitSignal() : Base::Signal(typeid(GameExitSignal)) {}
};

class GameToggleImGui : public Base::Signal {
public:
    GameToggleImGui() : Base::Signal(typeid(GameToggleImGui)) {}
};

class GameToggleDebugImGuiWindow : public Base::Signal {
public:
    GameToggleDebugImGuiWindow() : Base::Signal(typeid(GameToggleDebugImGuiWindow)) {}
};
