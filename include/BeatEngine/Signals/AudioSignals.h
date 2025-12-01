#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/AudioStream.h"

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
public:
	PlayAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), AudioStreamHandle(sound) {}
	PlayAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), AudioStreamHandle(sound) {}
};

class PauseAudioStreamSignal : public Base::Signal {
public:
	std::string AudioStreamName;
public:
	PauseAudioStreamSignal(std::string name) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(name) {}
	PauseAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(sound->GetName()) {}
	PauseAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(PauseAudioStreamSignal)), AudioStreamName(sound.Get()->GetName()) {}
};

class StopAudioStreamSignal : public Base::Signal {
public:
	std::string AudioStreamName;
public:
	StopAudioStreamSignal(std::string name) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(name) {}
	StopAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(sound->GetName()) {}
	StopAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), AudioStreamName(sound.Get()->GetName()) {}
};