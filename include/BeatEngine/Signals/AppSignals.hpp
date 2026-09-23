#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/AppFlags.hpp"
#include "BeatEngine/Enum/ViewFlags.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
// #include <SFML/Window/Cursor.hpp>

// class AppChangeCursorSignal : public Base::Signal {
// public:
//     sf::Cursor::Type NewCursor;
// public:
//     AppChangeCursorSignal(sf::Cursor::Type cursorType) 
//         : Base::Signal(typeid(AppChangeCursorSignal)), NewCursor(cursorType) {}
// };

class AppExitSignal : public Base::Signal {    
public:
    AppExitSignal() : Base::Signal(typeid(AppExitSignal)) {}
};

class AppToggleImGui : public Base::Signal {
public:
    AppToggleImGui() : Base::Signal(typeid(AppToggleImGui)) {}
};

class AppToggleDrawingDebugInfo : public Base::Signal {
public:
    AppToggleDrawingDebugInfo() : Base::Signal(typeid(AppToggleDrawingDebugInfo)) {}
};

class AppAddFlags : public Base::Signal {
public:
    AppFlags Flags;
public:
    AppAddFlags(AppFlags flags) : Base::Signal(typeid(AppAddFlags)), Flags(flags) {}
};

class AppRemoveFlags : public Base::Signal {
public:
    AppFlags Flags;
public:
    AppRemoveFlags(AppFlags flags) : Base::Signal(typeid(AppRemoveFlags)), Flags(flags) {}
};

class AppUninitGraphicsSignal : public Base::Signal {
public:
    GraphicalElement& Element;
public:
    AppUninitGraphicsSignal(GraphicalElement& element) : Element(element) {}
};

class ViewAddFlags : public Base::Signal {
public:
    ViewFlags Flags;
public:
    ViewAddFlags(ViewFlags flags) : Base::Signal(typeid(ViewAddFlags)), Flags(flags) {}
};

class ViewRemoveFlags : public Base::Signal {
public:
    ViewFlags Flags;
public:
    ViewRemoveFlags(ViewFlags flags) : Base::Signal(typeid(ViewRemoveFlags)), Flags(flags) {}
};
