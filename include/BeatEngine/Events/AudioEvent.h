#pragma once

#include "BeatEngine/Base/Event.h"
#include <string>

class EventAudioStreamStoped : public Base::Event {
public:
    String Name;
public:
    EventAudioStreamStoped(String name) : Base::Event(typeid(EventAudioStreamStoped)), Name(name) {}
};

class EventAudioStreamStarted : public Base::Event {
public:
    String Name;
public:
    EventAudioStreamStarted(String name) : Base::Event(typeid(EventAudioStreamStarted)), Name(name) {}
};

class EventSoundStarted : public Base::Event {
public:
    String Name;
public:
    EventSoundStarted(String name) : Base::Event(typeid(EventSoundStarted)), Name(name) {}
};

class EventSoundStoped : public Base::Event {
public:
    String Name;
public:
    EventSoundStoped(String name) : Base::Event(typeid(EventSoundStoped)), Name(name) {}
};

