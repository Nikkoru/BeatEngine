#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/AudioStream.h"

#include <memory>
#include <string>

class PlaySoundSignal : public Base::Signal {
public:
	std::shared_ptr<Sound> SoundToPlay;
public:
	PlaySoundSignal(std::shared_ptr<Sound> sound) : Base::Signal(typeid(PlaySoundSignal)), SoundToPlay(sound) {}
	PlaySoundSignal(Base::AssetHandle<Sound> sound) : Base::Signal(typeid(PlaySoundSignal)), SoundToPlay(sound.Get()) {}
};

class PlayAudioStreamSignal : public Base::Signal {
public:
	Base::AssetHandle<AudioStream> AudioStreamHandle;
    String AudioStreamName;
public:
	PlayAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), AudioStreamHandle(sound), AudioStreamName(sound->GetName()) {}
	PlayAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), AudioStreamHandle(sound), AudioStreamName() {}
	PlayAudioStreamSignal(String name) : Base::Signal(typeid(PlayAudioStreamSignal)), AudioStreamHandle(), AudioStreamName(name) {}
};

class PauseAudioStreamSignal : public Base::Signal {
public:
	String AudioStreamName;
public:
	PauseAudioStreamSignal(String name) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(name) {}
	PauseAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(sound->GetName()) {}
	PauseAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(sound.Get()->GetName()) {}
};

class StopAudioStreamSignal : public Base::Signal {
public:
	String AudioStreamName;
public:
	StopAudioStreamSignal(String name) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(name) {}
	StopAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(sound->GetName()) {}
	StopAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(sound.Get()->GetName()) {}
};
