#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/System/String.hpp"

struct EventAudioStreamStoped : Base::Event {
    String Name;

    EventAudioStreamStoped(String name) : Base::Event(typeid(EventAudioStreamStoped)), Name(name) {}
};

struct EventAudioStreamStarted : Base::Event {
    String Name;

    EventAudioStreamStarted(String name) : Base::Event(typeid(EventAudioStreamStarted)), Name(name) {}
};

struct EventSoundStarted : Base::Event {
    String Name;

    EventSoundStarted(String name) : Base::Event(typeid(EventSoundStarted)), Name(name) {}
};

struct EventSoundStoped : Base::Event {
    String Name;

    EventSoundStoped(String name) : Base::Event(typeid(EventSoundStoped)), Name(name) {}
};

