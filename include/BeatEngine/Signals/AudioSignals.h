#pragma once

#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Asset/Sound.h"
#include "BeatEngine/Asset/AudioStream.h"


class PlaySoundSignal : public Base::Signal {
public:
	std::shared_ptr<Sound> m_Sound;
public:
	PlaySoundSignal(std::shared_ptr<Sound> sound) : Base::Signal(typeid(PlaySoundSignal)), m_Sound(sound) {}
	PlaySoundSignal(Base::AssetHandle<Sound> sound) : Base::Signal(typeid(PlaySoundSignal)), m_Sound(sound.Get()) {}
};

class PlayAudioStreamSignal : public Base::Signal {
public:
	Base::AssetHandle<AudioStream> m_AudioStreamHandle;
public:
	PlayAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), m_AudioStreamHandle(sound) {}
	PlayAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(PlayAudioStreamSignal)), m_AudioStreamHandle(sound) {}
};

class StopAudioStreamSignal : public Base::Signal {
public:
	Base::AssetHandle<AudioStream> m_AudioStreamHandle;
public:
	StopAudioStreamSignal(std::shared_ptr<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), m_AudioStreamHandle(sound) {}
	StopAudioStreamSignal(Base::AssetHandle<AudioStream> sound) : Base::Signal(typeid(StopAudioStreamSignal)), m_AudioStreamHandle(sound) {}
};