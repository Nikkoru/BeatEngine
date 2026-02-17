#pragma once

#include "BeatEngine/Base/Event.h"
#include <string>

class EventAudioStreamStoped : public Base::Event {
public:
    std::string Name;
public:
    EventAudioStreamStoped(std::string name) : Base::Event(typeid(EventAudioStreamStoped)), Name(name) {}
};

class EventAudioStreamStarted : public Base::Event {
public:
    std::string Name;
public:
    EventAudioStreamStarted(std::string name) : Base::Event(typeid(EventAudioStreamStarted)), Name(name) {}
};

class EventSoundStarted : public Base::Event {
public:
    std::string Name;
public:
    EventSoundStarted(std::string name) : Base::Event(typeid(EventSoundStarted)), Name(name) {}
};

class EventSoundStoped : public Base::Event {
public:
    std::string Name;
public:
    EventSoundStoped(std::string name) : Base::Event(typeid(EventSoundStoped)), Name(name) {}
};

