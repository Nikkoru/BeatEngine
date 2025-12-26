#pragma once

#include <BeatEngine/Base/Event.h>
#include <string>

class AudioStreamStopedEvent : public Base::Event {
public:
    std::string Name;
public:
    AudioStreamStopedEvent(std::string name) : Base::Event(typeid(AudioStreamStopedEvent)), Name(name) {}
};

class AudioStreamStartedEvent : public Base::Event {
public:
    std::string Name;
public:
    AudioStreamStartedEvent(std::string name) : Base::Event(typeid(AudioStreamStartedEvent)), Name(name) {}
};

class SoundStartedEvent : public Base::Event {
public:
    std::string Name;
public:
    SoundStartedEvent(std::string name) : Base::Event(typeid(SoundStartedEvent)), Name(name) {}
};

class SoundStopedEvent : public Base::Event {
public:
    std::string Name;
public:
    SoundStopedEvent(std::string name) : Base::Event(typeid(SoundStopedEvent)), Name(name) {}
};

