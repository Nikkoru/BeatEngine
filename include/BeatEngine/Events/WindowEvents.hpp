#pragma once

#include "BeatEngine/Base/Event.h"

struct WindowResizedEvent : Base::Event {
    WindowResizedEvent() : Base::Event(typeid(WindowResizedEvent)) {}
};

struct WindowMovedEvent : Base::Event {
    WindowMovedEvent() : Base::Event(typeid(WindowMovedEvent)) {}
};

struct WindowMaximizedEvent : Base::Event {
    WindowMaximizedEvent() : Base::Event(typeid(WindowMaximizedEvent)) {}

};

struct WindowMinimizedEvent : Base::Event {
    WindowMinimizedEvent() : Base::Event(typeid(WindowMinimizedEvent)) {}
    
};

struct WindowFocusGainedEvent : Base::Event {
    WindowFocusGainedEvent() : Base::Event(typeid(WindowFocusGainedEvent)) {}

};

struct WindowFocusLostEvent : Base::Event {
    WindowFocusLostEvent() : Base::Event(typeid(WindowFocusLostEvent)) {}

};

struct WindowCloseRequestedEvent : Base::Event {
    WindowCloseRequestedEvent() : Base::Event(typeid(WindowCloseRequestedEvent)) {}
    
};

struct WindowEnterFullScreenEvent : Base::Event {
    WindowEnterFullScreenEvent() : Base::Event(typeid(WindowEnterFullScreenEvent)) {}

};

struct WindowExitFullScreenEvent : Base::Event {
    WindowExitFullScreenEvent() : Base::Event(typeid(WindowExitFullScreenEvent)) {}
};
