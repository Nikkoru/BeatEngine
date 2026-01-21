#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/GameFlags.h"
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

class GameToggleDrawingDebugInfo : public Base::Signal {
public:
    GameToggleDrawingDebugInfo() : Base::Signal(typeid(GameToggleDrawingDebugInfo)) {}
};

class GameAddFlags : public Base::Signal {
public:
    GameFlags Flags;
public:
    GameAddFlags(GameFlags flags) : Base::Signal(typeid(GameAddFlags)), Flags(flags) {}
};

class GameRemoveFlags : public Base::Signal {
public:
    GameFlags Flags;
public:
    GameRemoveFlags(GameFlags flags) : Base::Signal(typeid(GameRemoveFlags)), Flags(flags) {}
};
