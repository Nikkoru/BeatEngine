#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Windows/Mouse.hpp"

class MouseMovedEvent : public Base::Event {
public:
    Vector2i Position;
public:
    MouseMovedEvent(Vector2i pos) : Base::Event(typeid(MouseMovedEvent)), Position(pos) {}
};

class MouseButtonDownEvent : public Base::Event {
public:
    Mouse::Button Button;
    Vector2i Position;
public:
    MouseButtonDownEvent(Mouse::Button button, Vector2i pos) : Base::Event(typeid(MouseButtonDownEvent)), Button(button), Position(pos) {}
};

class MouseButtonUpEvent : public Base::Event {
public:
    Mouse::Button Button;
    Vector2i Position;
public:
    MouseButtonUpEvent(Mouse::Button button, Vector2i pos) : Base::Event(typeid(MouseButtonUpEvent)),Button(button), Position(pos) {}
};
